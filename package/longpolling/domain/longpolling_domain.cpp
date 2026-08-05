//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "longpolling_domain.hpp"
#include <longpolling/longpolling.hpp>
#include <pubsub/api/subscribe.hpp>
#include <longpolling/logger.hpp>
#include <chrono>

namespace wfc{ namespace pubsub{

  
longpolling_domain::~longpolling_domain()
{
}

void longpolling_domain::configure()
{
  _longpolling = std::make_shared<longpolling>( this->options() );
  _subscribe_batch = this->options().subscribe_batch;
  _hub_inflight.configure(
    this->options().subscribe_inflight_max,
    this->options().describe_inflight_max,
    this->options().inflight_timeout_s
  );
  _auth.bind(this->options().auth);
}

void longpolling_domain::initialize()
{
  _wpubsub = this->get_target<ipubsub>(this->options().target);
}

void longpolling_domain::start()
{
  _auth.bind(this->options().auth);
  _auth.start();

  if ( auto st = this->get_statistics() )
    _stat.start(*st);

  _fire_timer = this->get_workflow()->create_timer(
    std::chrono::milliseconds(this->options().fire_timer_ms),
    [wself = std::weak_ptr<longpolling_domain>(this->shared_from_this())]() -> bool
    {
      if ( auto pthis = wself.lock() )
        return pthis->longpolling_fire_();
      return false;
    }
  );
  auto puuid = std::make_shared<std::string>();
  std::weak_ptr<ipubsub> wpubsub = _wpubsub;
  std::weak_ptr<longpolling> wlongpoll = _longpolling;
  std::weak_ptr<longpolling_domain> wdomain = this->shared_from_this();
  io_id_t io_id = this->get_id();
  _ping_timer = this->get_workflow()->create_requester<request::ping, response::ping>(
    std::chrono::milliseconds(this->options().ping_timer_ms),
    [ wpubsub, io_id](request::ping::ptr req, response::ping::callback cb)->bool
    {
      if ( auto pps = wpubsub.lock() )
      {
        pps->ping(std::move(req), cb, io_id);
      }
      return true;
    },
    [puuid, wlongpoll, wdomain](response::ping::ptr res) mutable ->request::ping::ptr
    {
      if ( res != nullptr )
      {
        if ( puuid->empty() )
          *puuid = res->uuid;

        if ( *puuid != res->uuid )
        {
          if (auto plp = wlongpoll.lock() )
          {
            LONGPOLL_LOG_WARNING("pubsub UUID has been changed. Oversubscription to all channels.")
            *puuid = res->uuid;
            plp->resubscribe();
          }
          if ( auto pdomain = wdomain.lock() )
            pdomain->reset_hub_inflight_();
        }
        return nullptr;
      }
      return std::make_unique<request::ping>();
    }
  );
  _auth.ensure_subscribed(*_longpolling);
}

void longpolling_domain::stop()
{
  if ( _longpolling != nullptr && _auth.configured() )
  {
    _auth.unsubscribe(*_longpolling);
    this->make_descriptions_();
  }

  this->reset_hub_inflight_();
  this->get_workflow()->release_timer(_fire_timer);
  this->get_workflow()->release_timer(_ping_timer);
}


void longpolling_domain::create1( request::create::ptr req, response::create::callback cb )
{
  if ( this->notify_ban(req, cb) )
    return;

  if ( _auth.enabled() )
  {
    const auth_status st = _auth.check_credentials(req->oid, req->sid);
    if ( st != auth_status::ok )
    {
      _auth.note_unauthorized(st);
      _auth.log_create_unauthorized(req->oid, req->sid, st);
      if ( !_auth.suspend() )
      {
        cb(nullptr);
        return;
      }
    }
    else
    {
      _auth.clear_fail_log(req->oid);
    }
  }

  if ( req->uuid.empty() )
    req->uuid = agent::create_uuid();

  auto res = this->create_response(cb);

  if ( res != nullptr )
    res->uuid = req->uuid;

  agent_options ao = static_cast<const agent_options>(*req);
  if ( _auth.enabled() )
  {
    ao.oid = req->oid;
    if ( ao.key == 0 && req->oid != 0 )
      ao.key = req->oid;
  }

  if ( !_longpolling->create( ao ) )
  {
    LONGPOLL_LOG_ERROR("Ошибка! Повторное создание с другим ключом!")
  }
  else
  {
    agent_params ap;
    ap.uuid = req->uuid;
    ap.key = ao.key;
    ap.oid = req->oid;
    ap.sid = req->sid;
    ap.confirm = false;
    _longpolling->open(ap, req->channels);
  }
  cb( std::move(res) );
}

void longpolling_domain::open( request::open::ptr req, response::open::callback cb )
{
  if ( this->bad_request(req, cb) )
    return;

  if ( _auth.enabled() )
  {
    const auth_status st = _auth.check_agent_access(*_longpolling, req->uuid, req->oid, req->sid);
    if ( st != auth_status::ok )
    {
      _auth.note_unauthorized(st);
      LONGPOLL_LOG_WARNING("open: unauthorized uuid=" << req->uuid
                           << " oid=" << req->oid << " sid=" << req->sid
                           << " reason=" << auth_status_str(st))
      if ( !_auth.suspend() )
      {
        cb(nullptr);
        return;
      }
    }
  }

  auto res = this->create_response(cb);

  _longpolling->open(*req, req->channels);

  this->send_response(std::move(res), cb);
}

void longpolling_domain::close( request::close::ptr req, response::close::callback cb )
{
  if ( this->bad_request(req, cb) )
    return;

  if ( _auth.enabled() )
  {
    const auth_status st = _auth.check_agent_access(*_longpolling, req->uuid, req->oid, req->sid);
    if ( st != auth_status::ok )
    {
      _auth.note_unauthorized(st);
      LONGPOLL_LOG_WARNING("close: unauthorized uuid=" << req->uuid
                           << " oid=" << req->oid << " sid=" << req->sid
                           << " reason=" << auth_status_str(st))
      if ( !_auth.suspend() )
      {
        cb(nullptr);
        return;
      }
    }
  }

  auto res = this->create_response(cb);

  _longpolling->close(req->uuid, req->channels);

  this->send_response(std::move(res), cb);
}

void longpolling_domain::longpoll( request::longpoll::ptr req, response::longpoll::callback cb )
{
  if ( this->notify_ban(req, cb) )
    return;

  if ( _auth.enabled() )
  {
    const auth_status st = _auth.check_agent_access(*_longpolling, req->uuid, req->oid, req->sid);
    if ( st != auth_status::ok )
    {
      _auth.note_unauthorized(st);
      LONGPOLL_LOG_WARNING("longpoll: unauthorized uuid=" << req->uuid
                           << " oid=" << req->oid << " sid=" << req->sid
                           << " reason=" << auth_status_str(st))
      if ( !_auth.suspend() )
      {
        cb(nullptr);
        return;
      }
    }
  }

  if ( !_longpolling->longpoll(*req, [cb](const topic_list_t& tl)
  {
    auto res = std::make_unique<response::longpoll>();
    res->messages = topic::copy_list(tl);
    cb( std::move(res) );
  }) )
  {
    cb(nullptr);
  }
}

void longpolling_domain::publish( request::publish::ptr req, response::publish::callback cb )
{
  if ( this->bad_request(req, cb) )
    return;

  if ( _auth.enabled() )
  {
    const auth_status st = _auth.check_session_with_grace(req->oid, req->sid);
    if ( st != auth_status::ok )
    {
      _auth.note_unauthorized(st);
      LONGPOLL_LOG_WARNING("publish: unauthorized oid=" << req->oid << " sid=" << req->sid
                           << " reason=" << auth_status_str(st))
      if ( !_auth.suspend() )
      {
        cb(nullptr);
        return;
      }
    }
  }

  if ( !this->options().publish.allows_all(req->messages) )
  {
    LONGPOLL_LOG_WARNING("publish: client publish disabled or channel not allowed")
    cb(nullptr);
    return;
  }

  if ( auto pubsub = _wpubsub.lock() )
  {
    pubsub->publish(std::move(req), cb );
  }
  else
  {
    if ( auto res = this->create_response(cb) )
    {
      cb( std::move(res) );
    }
  }
}

// isubscriber
void longpolling_domain::notify( request::publish::ptr req, response::publish::callback cb )
{
  if ( this->bad_request(req, cb) )
    return;

  auto res = this->create_response(cb);

  for (const auto& m: req->messages)
  {
    this->handle_hub_message_(m);
  }

  this->send_response( std::move(res), cb);
}

void longpolling_domain::reg_io(io_id_t id, std::weak_ptr<iinterface> itf)
{
  super::reg_io(id, itf);
  LONGPOLL_LOG_DEBUG("longpolling_domain::reg_io " << id);
}

void longpolling_domain::unreg_io(io_id_t id)
{
  super::unreg_io(id);
  LONGPOLL_LOG_DEBUG("longpolling_domain::unreg_io " << id);
}

bool longpolling_domain::longpolling_fire_()
{
  try{
  static time_t firelog_s = 0;
  static fire_stat stat;
  static size_t subscribe_count = 0;
  static size_t describe_count = 0;
  if ( _auth.configured() )
  {
    _auth.expire();
  }
  if ( firelog_s == 0)
  {
    firelog_s=this->options().fire_log_s;
  }
  // Очередь на отписку формируется с учетом очереди на отписку, поэтому ее сначала
  describe_count += this->make_descriptions_();
  subscribe_count += this->make_subscriptions_();
  // Забрать для подписки и отписки
  fire_stat cur_stat;
  _longpolling->fire(&cur_stat);

  if ( _stat.enabled() && this->get_statistics() )
  {
    _stat.write(
      cur_stat,
      describe_count,
      subscribe_count,
      _auth.take_error_stat(*_longpolling)
    );
  }
  stat+=cur_stat;
  static std::atomic<time_t> timelog = time(nullptr);
  time_t now = time(nullptr);
  if ( now - timelog > firelog_s )
  {
    timelog = time(nullptr);
    if ( _auth.configured() )
    {
      _auth.fire_log(*_longpolling);
    }
    if ( subscribe_count > 0 || _hub_inflight.subscribe_count() > 0 )
    {
      hub_queue_stat qs = _longpolling->get_hub_queue_stat();
      size_t queue = qs.wait_subscribe + qs.inflight_subscribe;
      LONGPOLL_LOG_MESSAGE(
        "Subscribe drain period " << firelog_s << "s: sent=" << subscribe_count
        << " inflight=" << _hub_inflight.subscribe_count() << "/" << _hub_inflight.subscribe_max()
        << " queue=" << queue
        << " (wait=" << qs.wait_subscribe << " inflight=" << qs.inflight_subscribe << ")"
      );
    }
    if ( describe_count > 0 || _hub_inflight.describe_count() > 0 )
    {
      hub_queue_stat qs = _longpolling->get_hub_queue_stat();
      size_t queue = qs.wait_describe + qs.inflight_describe;
      LONGPOLL_LOG_MESSAGE(
        "Describe drain period " << firelog_s << "s: sent=" << describe_count
        << " inflight=" << _hub_inflight.describe_count() << "/" << _hub_inflight.describe_max()
        << " queue=" << queue
        << " (wait=" << qs.wait_describe << " inflight=" << qs.inflight_describe << ")"
      );
    }
    stat = fire_stat();
    subscribe_count = 0;
    describe_count = 0;
    firelog_s=this->options().fire_log_s;
  }
  }catch(const std::exception& e)
  {
    LONGPOLL_LOG_ERROR("longpolling_fire_ exception: " << e.what() )
  }
  catch(...)
  {
    LONGPOLL_LOG_ERROR("longpolling_fire_ exception: ...")
  }
  return true;
}

size_t longpolling_domain::make_subscriptions_()
{
  _hub_inflight.recover();

  hub_queue_stat q_before = _longpolling->get_hub_queue_stat();
  size_t queue_before = q_before.wait_subscribe + q_before.inflight_subscribe;

  size_t subscribe_count = 0;
  size_t sent_batches = 0;
  bool throttled = false;

  for (;;)
  {
    if ( _hub_inflight.subscribe_full() )
    {
      throttled = true;
      break;
    }

    auto subs_channels = std::make_shared<std::vector<std::string>>();
    bool has_more = _longpolling->pop_for_subscribe( subs_channels.get(), _subscribe_batch );
    if ( subs_channels->empty() )
      break;

    subscribe_count += subs_channels->size();
    ++sent_batches;

    if ( auto pubsub = _wpubsub.lock() )
    {
      auto req = std::make_unique<request::subscribe>();
      for (const std::string& ch : *subs_channels)
      {
        subscribe_params sp;
        sp.channel = ch;
        sp.cursor = 0;
        sp.limit = 1000;
        req->channels.push_back(sp);
      }

      std::weak_ptr<longpolling> wpolling = _longpolling;
      std::weak_ptr<longpolling_domain> wself = this->shared_from_this();
      size_t log_batch = has_more ? sent_batches : 0;

      _hub_inflight.note_subscribe();

      response::subscribe::callback cb = this->callback([wself, wpolling, subs_channels, log_batch](response::subscribe::ptr res)
      {
        if ( auto pdomain = wself.lock() )
          pdomain->on_subscribe_response_(*subs_channels, log_batch, std::move(res));
        else if ( res == nullptr )
        {
          if ( auto ppolling = wpolling.lock() )
          {
            LONGPOLL_LOG_WARNING("Subscribe rollback" );
            ppolling->rollback_for_subscribe(*subs_channels);
          }
        }
      }); // cb = this->callback

      pubsub->subscribe( std::move(req), cb, this->get_id(), this->shared_from_this() );
    }
    else
    {
      LONGPOLL_LOG_WARNING("Subscribe rollback: pubsub unavailable" );
      _longpolling->rollback_for_subscribe(*subs_channels);
    }

    if ( !has_more )
      break;
  }

  hub_queue_stat q_after = _longpolling->get_hub_queue_stat();
  size_t queue_after = q_after.wait_subscribe + q_after.inflight_subscribe;
  const size_t inflight = _hub_inflight.subscribe_count();
  const bool draining = sent_batches > 0 || queue_after > 0 || queue_before > 0;

  static auto last_drain_log = std::chrono::steady_clock::time_point{};
  const auto log_now = std::chrono::steady_clock::now();
  const bool may_log_drain = (log_now - last_drain_log) >= std::chrono::seconds(1);

  if ( may_log_drain && draining && (sent_batches > 0 || queue_after > 0 || throttled) )
  {
    if ( throttled && queue_after > 0 )
    {
      LONGPOLL_LOG_WARNING(
        "Subscribe drain throttled: inflight=" << inflight << "/" << _hub_inflight.subscribe_max()
        << " backlog=" << queue_after
        << " +channels=" << subscribe_count
        << " batches=" << sent_batches
      );
    }
    else
    {
      LONGPOLL_LOG_MESSAGE(
        "Subscribe drain: +channels=" << subscribe_count
        << " batches=" << sent_batches
        << " inflight=" << inflight << "/" << _hub_inflight.subscribe_max()
        << " queue=" << queue_after
        << " (wait=" << q_after.wait_subscribe << " inflight=" << q_after.inflight_subscribe << ")"
      );
    }
    last_drain_log = log_now;
  }

  static size_t prev_queue = 0;
  if ( prev_queue > _subscribe_batch && queue_after == 0 && inflight == 0 )
  {
    LONGPOLL_LOG_MESSAGE("Subscribe backlog drained");
  }
  prev_queue = queue_after;

  return subscribe_count;
}

size_t longpolling_domain::make_descriptions_()
{
  _hub_inflight.recover();

  hub_queue_stat q_before = _longpolling->get_hub_queue_stat();
  size_t queue_before = q_before.wait_describe + q_before.inflight_describe;

  size_t describe_count = 0;
  size_t sent_batches = 0;
  bool throttled = false;

  for (;;)
  {
    if ( _hub_inflight.describe_full() )
    {
      throttled = true;
      break;
    }

    auto desc_channels = std::make_shared<std::vector<std::string>>();
    bool has_more = _longpolling->pop_for_describe( desc_channels.get(), _subscribe_batch );
    if ( desc_channels->empty() )
      break;

    describe_count += desc_channels->size();
    ++sent_batches;

    if ( auto pubsub = _wpubsub.lock() )
    {
      auto req = std::make_unique<request::describe>();
      for (const std::string& ch : *desc_channels)
        req->channels.push_back(ch);

      std::weak_ptr<longpolling> wpolling = _longpolling;
      std::weak_ptr<longpolling_domain> wself = this->shared_from_this();
      size_t log_batch = has_more ? sent_batches : 0;

      _hub_inflight.note_describe();

      response::describe::callback cb = this->callback([wself, wpolling, desc_channels, log_batch](response::describe::ptr res)
      {
        if ( auto pdomain = wself.lock() )
          pdomain->on_describe_response_(*desc_channels, log_batch, std::move(res));
        else if ( res == nullptr )
        {
          if ( auto ppolling = wpolling.lock() )
          {
            LONGPOLL_LOG_WARNING("Describe rollback" );
            ppolling->rollback_for_describe(*desc_channels);
          }
        }
      });

      pubsub->describe( std::move(req), cb, this->get_id() );
    }
    else
    {
      LONGPOLL_LOG_WARNING("Describe rollback: pubsub unavailable" );
      _longpolling->rollback_for_describe(*desc_channels);
    }

    if ( !has_more )
      break;
  }

  hub_queue_stat q_after = _longpolling->get_hub_queue_stat();
  size_t queue_after = q_after.wait_describe + q_after.inflight_describe;
  const size_t inflight = _hub_inflight.describe_count();
  const bool draining = sent_batches > 0 || queue_after > 0 || queue_before > 0;

  static auto last_drain_log = std::chrono::steady_clock::time_point{};
  const auto log_now = std::chrono::steady_clock::now();
  const bool may_log_drain = (log_now - last_drain_log) >= std::chrono::seconds(1);

  if ( may_log_drain && draining && (sent_batches > 0 || queue_after > 0 || throttled) )
  {
    if ( throttled && queue_after > 0 )
    {
      LONGPOLL_LOG_WARNING(
        "Describe drain throttled: inflight=" << inflight << "/" << _hub_inflight.describe_max()
        << " backlog=" << queue_after
        << " +channels=" << describe_count
        << " batches=" << sent_batches
      );
    }
    else
    {
      LONGPOLL_LOG_MESSAGE(
        "Describe drain: +channels=" << describe_count
        << " batches=" << sent_batches
        << " inflight=" << inflight << "/" << _hub_inflight.describe_max()
        << " queue=" << queue_after
        << " (wait=" << q_after.wait_describe << " inflight=" << q_after.inflight_describe << ")"
      );
    }
    last_drain_log = log_now;
  }

  static size_t prev_queue = 0;
  if ( prev_queue > _subscribe_batch && queue_after == 0 && inflight == 0 )
  {
    LONGPOLL_LOG_MESSAGE("Describe backlog drained");
  }
  prev_queue = queue_after;

  return describe_count;
}

void longpolling_domain::handle_hub_message_(const topic& m)
{
  if ( _auth.try_handle_hub_message(m) )
    return;
  _longpolling->push(m.channel, m);
}

void longpolling_domain::on_subscribe_response_(
  const std::vector<std::string>& subs_channels,
  size_t batch_count,
  response::subscribe::ptr res)
{
  _hub_inflight.ack_subscribe();

  if ( res != nullptr )
  {
    _longpolling->confirm_for_subscribe(subs_channels);
    if ( batch_count > 0 )
    {
      LONGPOLL_LOG_MESSAGE("Subscribe batch №" << batch_count << " has " << res->messages.size() << " messages")
    }
    for ( const auto& m : res->messages )
      this->handle_hub_message_(m);
  }
  else
  {
    LONGPOLL_LOG_WARNING("Subscribe rollback" );
    _longpolling->rollback_for_subscribe(subs_channels);
  }
}

void longpolling_domain::on_describe_response_(
  const std::vector<std::string>& desc_channels,
  size_t batch_count,
  response::describe::ptr res)
{
  _hub_inflight.ack_describe();

  if ( res != nullptr )
  {
    _longpolling->confirm_for_describe(desc_channels);
    if ( batch_count > 0 )
    {
      LONGPOLL_LOG_MESSAGE("Describe batch №" << batch_count << " ready")
    }
  }
  else
  {
    LONGPOLL_LOG_WARNING("Describe rollback" );
    _longpolling->rollback_for_describe(desc_channels);
  }
}

void longpolling_domain::reset_hub_inflight_()
{
  _hub_inflight.reset();
}

}}
