//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "longpolling_domain.hpp"
#include <longpolling/longpolling.hpp>
#include <pubsub/api/subscribe.hpp>
#include <longpolling/logger.hpp>

namespace wfc{ namespace pubsub{

  /*
class longpolling_domain::subscriber
  : public isubscriber
{

public:
  subscriber(std::weak_ptr<longpolling> wlp )
    : _wlongpolling(wlp)
  {}

  void publish( request::publish::ptr req, response::publish::callback cb )
  {
    if ( req==nullptr )
    {
      if ( cb!=nullptr) cb(nullptr);
      return;
    }

    if ( auto plongpolling = _wlongpolling.lock() )
    {
      for (const auto& m: req->messages)
      {
        plongpolling->push(m.channel, m);
      }
    }

    if ( cb!=nullptr )
    {
      auto res = std::make_unique<response::publish>();
      cb( std::move(res) );
    }
  }
private:
  std::weak_ptr<longpolling> _wlongpolling;
};
*/
  
longpolling_domain::~longpolling_domain()
{
}

void longpolling_domain::configure()
{
  _longpolling = std::make_shared<longpolling>( this->options() );
  _subscribe_batch = this->options().subscribe_batch;
}

void longpolling_domain::initialize()
{
  _wpubsub = this->get_target<ipubsub>(this->options().target);
}

void longpolling_domain::start()
{
  if ( auto st = this->get_statistics() )
  {
    _fire_meters = {
      // 0. Все отправленные сообщения
      st->create_value_meter("sended_messages"),
      // 1. Сообщения ожидающие подверждения о доставке
      st->create_value_meter("wait_messages"),
      // 2. Сообщения хранимые в агентах
      st->create_value_meter("stored_messages"),
      // 3. Удалено устаревших сообщений
      st->create_value_meter("remove_death"),
      // 4. все каналы со всех агентов включая дубликаты
      st->create_value_meter("active_channels"),
      // 5. Активные агенты
      st->create_value_meter("active_agents"),
      // 6. Удаленные агенты
      st->create_value_meter("deleted_agents"),
      // 7. Сообщения хранимые в агентах
      st->create_value_meter("hub_stored_messages"),
      // 8. все каналы со всех агентов включая дубликаты
      st->create_value_meter("hub_active_channels"),
      // 9. Удалено устаревших сообщений из общего хаба
      st->create_value_meter("hub_remove_death"),
      // 10.
      st->create_value_meter("counter_map"),
      // 11.
      st->create_value_meter("wait_subscribe"),
      // 12.
      st->create_value_meter("wait_describe"),
      // 13.
      st->create_value_meter("describe_delay"),
      // 14.
      st->create_value_meter("lost_subscribe"),
      // 15.
      st->create_value_meter("lost_describe"),
      // 16.
      st->create_value_meter("describe_count"),
      // 17.
      st->create_value_meter("subscribe_count"),
      // 18.
      st->create_value_meter("active_agents_uc"),
      // 19.
      st->create_value_meter("dead_describe")
    };
  }

  _fire_timer = this->get_workflow()->create_timer(
    std::chrono::milliseconds(this->options().fire_timer_ms),
    std::bind(&longpolling_domain::longpolling_fire_, this)
  );
  auto puuid = std::make_shared<std::string>();
  std::weak_ptr<ipubsub> wpubsub = _wpubsub;
  std::weak_ptr<longpolling> wlongpoll = _longpolling;
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
    [puuid, wlongpoll](response::ping::ptr res) mutable ->request::ping::ptr
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
        }
        return nullptr;
      }
      return std::make_unique<request::ping>();
    }
  );
}

void longpolling_domain::stop()
{
  this->get_workflow()->release_timer(_fire_timer);
  this->get_workflow()->release_timer(_ping_timer);
}


void longpolling_domain::create1( request::create::ptr req, response::create::callback cb )
{
  if ( this->notify_ban(req, cb) )
    return;

  auto res = this->create_response(cb);

  if ( req->uuid.empty() )
    req->uuid = agent::create_uuid();
  res->uuid = req->uuid;

  if ( !_longpolling->create( static_cast<const agent_options>(*req) ) )
  {
    LONGPOLL_LOG_ERROR("Ошибка! Повторное создание с другим ключом!")
  }
  else
  {
    agent_params ap;
    ap.uuid = res->uuid;
    ap.key = req->key;
    ap.confirm = false;
    _longpolling->open(ap, req->channels);
  }
  cb( std::move(res) );
}

void longpolling_domain::create_depr( depr::request::create::ptr req, depr::response::create::callback cb)
{
  if ( this->bad_request(req, cb) )
    return;

  return this->create1(std::move(req), [cb, this](response::create::ptr res)
  {
    auto uuid = this->create_response(cb);

    uuid->uuid = res->uuid;

    this->send_response(std::move(uuid), cb);

  });
}


void longpolling_domain::open( request::open::ptr req, response::open::callback cb )
{
  if ( this->bad_request(req, cb) )
    return;

  auto res = this->create_response(cb);

  _longpolling->open(*req, req->channels);

  this->send_response(std::move(res), cb);
}

void longpolling_domain::close( request::close::ptr req, response::close::callback cb )
{
  if ( this->bad_request(req, cb) )
    return;

  auto res = this->create_response(cb);

  _longpolling->close(req->uuid, req->channels);

  this->send_response(std::move(res), cb);
}

void longpolling_domain::longpoll( request::longpoll::ptr req, response::longpoll::callback cb )
{
  if ( this->notify_ban(req, cb) )
    return;

  _longpolling->longpoll(*req, [cb](const topic_list_t& tl)
  {
    auto res = std::make_unique<response::longpoll>();
    res->messages = topic::copy_list(tl);
    cb( std::move(res) );
  });
}

// isubscriber
void longpolling_domain::publish( request::publish::ptr req, response::publish::callback cb )
{
  if ( this->bad_request(req, cb) )
    return;

  if ( auto pubsub = _wpubsub.lock() )
  {
    pubsub->publish(std::move(req), cb );
  }
  else
  {
    if ( auto res = this->create_response(cb) )
    {
      // TODO: res->status = Bad Gateway
      cb( std::move(res) );
    }
  }
}

void longpolling_domain::notify( request::publish::ptr req, response::publish::callback cb )
{
  if ( this->bad_request(req, cb) )
    return;

  auto res = this->create_response(cb);

  for (const auto& m: req->messages)
  {
    _longpolling->push(m.channel, m);
  }

  this->send_response( std::move(res), cb);
}

void longpolling_domain::subscribe(
    request::longpoll::ptr req, depr::response::subscribe::callback cb,
    io_id_t , std::weak_ptr<depr::icomet_subscriber> wcs)
{

  if ( cb != nullptr )
  {
    this->longpoll(std::move(req), [cb]( response::longpoll::ptr res)
    {
      typedef depr::response::subscribe::message_list message_list;
      cb( std::make_unique<message_list>( std::move(res->messages) ));
    });
  }
  else
  {
    this->longpoll(std::move(req), [wcs]( response::longpoll::ptr res)
    {
      if ( auto pcs = wcs.lock() )
      {
        typedef depr::request::subscribe::message_list message_list;
        pcs->subscribe( std::make_unique<message_list>( std::move(res->messages) ), nullptr);
      }
    });
  }
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

  if ( !_fire_meters.empty() && this->get_statistics() )
  {
    _fire_meters.at(0).create(static_cast<wrtstat::value_type>(cur_stat.sended_messages), 0ul);
    _fire_meters.at(1).create(static_cast<wrtstat::value_type>(cur_stat.wait_messages), 0ul);
    _fire_meters.at(2).create(static_cast<wrtstat::value_type>(cur_stat.stored_messages), 0ul);
    _fire_meters.at(3).create(static_cast<wrtstat::value_type>(cur_stat.remove_death), 0ul);
    _fire_meters.at(4).create(static_cast<wrtstat::value_type>(cur_stat.active_channels), 0ul);
    _fire_meters.at(5).create(static_cast<wrtstat::value_type>(cur_stat.active_agents), 0ul);
    _fire_meters.at(6).create(static_cast<wrtstat::value_type>(cur_stat.deleted_agents), 0ul);
    _fire_meters.at(7).create(static_cast<wrtstat::value_type>(cur_stat.hub_stored_messages), 0ul);
    _fire_meters.at(8).create(static_cast<wrtstat::value_type>(cur_stat.hub_active_channels), 0ul);
    _fire_meters.at(9).create(static_cast<wrtstat::value_type>(cur_stat.hub_remove_death), 0ul);
    _fire_meters.at(10).create(static_cast<wrtstat::value_type>(cur_stat.subs_stat.counter_map), 0ul);
    _fire_meters.at(11).create(static_cast<wrtstat::value_type>(cur_stat.subs_stat.wait_subscribe), 0ul);
    _fire_meters.at(12).create(static_cast<wrtstat::value_type>(cur_stat.subs_stat.wait_describe), 0ul);
    _fire_meters.at(13).create(static_cast<wrtstat::value_type>(cur_stat.subs_stat.describe_delay), 0ul);
    _fire_meters.at(14).create(static_cast<wrtstat::value_type>(cur_stat.subs_stat.lost_subscribe), 0ul);
    _fire_meters.at(15).create(static_cast<wrtstat::value_type>(cur_stat.subs_stat.lost_describe), 0ul);
    _fire_meters.at(16).create(static_cast<wrtstat::value_type>(describe_count), 0ul);
    _fire_meters.at(17).create(static_cast<wrtstat::value_type>(subscribe_count), 0ul);
    _fire_meters.at(18).create(static_cast<wrtstat::value_type>(cur_stat.active_agents_uc), 0ul);
    _fire_meters.at(19).create(static_cast<wrtstat::value_type>(cur_stat.dead_describe), 0ul);

  }
  stat+=cur_stat;
  static std::atomic<time_t> timelog = time(nullptr);
  time_t now = time(nullptr);
  if ( now - timelog > firelog_s )
  {
    timelog = time(nullptr);
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

auto longpolling_domain::pop_for_(bool (longpolling::*poll)(std::vector<std::string>*,size_t))
{

  typedef std::vector<std::string> vector_of_string;
  typedef std::shared_ptr<vector_of_string> channel_list_ptr;
  typedef std::list<channel_list_ptr> list_of_list_t;

  // channel_list_ptr subs_channels = std::make_shared<vector_of_string>();

  list_of_list_t subscribe_list;
  for (;;)
  {
    subscribe_list.push_back(std::make_shared<vector_of_string>());
    if ( !(_longpolling.get()->*poll)( subscribe_list.back().get(), _subscribe_batch ) )
      break;
  }
  return subscribe_list;
}


size_t longpolling_domain::make_subscriptions_()
{
  auto subscribe_list = this->pop_for_(&longpolling::pop_for_subscribe);

  size_t subscribe_count = 0;
  size_t batch_count = subscribe_list.size();
  if ( batch_count > 1)
  {
    LONGPOLL_LOG_MESSAGE("Ready subscribe batches: " << batch_count)
  }
  else
  {
    batch_count = 0; // Не будем логировать
  }
  for ( auto subs_channels : subscribe_list)
  {
    if ( !subs_channels->empty() )
    {
      subscribe_count += subs_channels->size();
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

        response::subscribe::callback cb = this->callback([subs_channels, wpolling, batch_count](response::subscribe::ptr res)
        {
          if ( res != nullptr )
          {
            if ( auto ppolling = wpolling.lock() )
              ppolling->confirm_for_subscribe(*subs_channels);
            if ( batch_count > 0 )
            {
              LONGPOLL_LOG_MESSAGE("Subscribe batch №" << batch_count << " has " << res->messages.size() << " messages")
            }
            for ( auto& m : res->messages )
            {
              if ( auto ppolling = wpolling.lock() )
              {
                ppolling->push(m.channel, m);
              }
            }
          }
          else
          {
            if ( auto ppolling = wpolling.lock() )
            {
              LONGPOLL_LOG_WARNING("Subscribe rollback" );
              ppolling->rollback_for_subscribe(*subs_channels);
            }
          }
        }); // cb = this->callback

        --batch_count;
        pubsub->subscribe( std::move(req), cb, this->get_id(), this->shared_from_this() );
      }
    }
  } //for
  return subscribe_count;
}

size_t longpolling_domain::make_descriptions_()
{
  auto describe_list = this->pop_for_(&longpolling::pop_for_describe);

  size_t describe_count = 0;
  size_t batch_count = describe_list.size();
  if ( batch_count > 1)
  {
    LONGPOLL_LOG_MESSAGE("Ready describe batches: " << batch_count)
  }
  else
  {
    batch_count = 0; // Не будем логировать
  }
  for ( auto subs_channels : describe_list)
  {
    if ( !subs_channels->empty() )
    {
      describe_count += subs_channels->size();
      if ( auto pubsub = _wpubsub.lock() )
      {
        auto req = std::make_unique<request::describe>();
        for (const std::string& ch : *subs_channels)
        {
          req->channels.push_back(ch);
        }

        std::weak_ptr<longpolling> wpolling = _longpolling;

        response::describe::callback cb = this->callback([subs_channels, wpolling, batch_count](response::describe::ptr res)
        {
          if ( res != nullptr )
          {
            if ( auto ppolling = wpolling.lock() )
            {
              ppolling->confirm_for_describe(*subs_channels);
            }

            if ( batch_count > 0 )
            {
              LONGPOLL_LOG_MESSAGE("Describe batch №" << batch_count << " ready")
            }
          }
          else
          {
            if ( auto ppolling = wpolling.lock() )
            {
              LONGPOLL_LOG_WARNING("Describe rollback" );
              ppolling->rollback_for_describe(*subs_channels);
            }
          }
        }); // cb = this->callback

        pubsub->describe( std::move(req), cb, this->get_id() );
      }
    }
  } //for
  return describe_count;
}

}}
