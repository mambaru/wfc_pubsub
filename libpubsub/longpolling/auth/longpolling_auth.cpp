//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "longpolling_auth.hpp"
#include <longpolling/longpolling.hpp>
#include <longpolling/auth/auth_session_json.hpp>
#include <longpolling/logger.hpp>

namespace wfc{ namespace pubsub{

namespace {

const auth_options& empty_auth_options()
{
  static const auth_options opt;
  return opt;
}

} // namespace

void longpolling_auth::bind(const auth_options& opt)
{
  _opt = &opt;
}

void longpolling_auth::start()
{
  _started_at = std::chrono::steady_clock::now();
}

bool longpolling_auth::configured() const
{
  const auto& auth = _opt != nullptr ? *_opt : empty_auth_options();
  return !auth.disabled && !auth.channel.empty();
}

bool longpolling_auth::enabled() const
{
  return this->configured();
}

bool longpolling_auth::suspend() const
{
  return _opt != nullptr && _opt->suspend;
}

bool longpolling_auth::in_grace_period() const
{
  const time_t grace = _opt != nullptr ? _opt->grace_period_s : 0;
  if ( grace <= 0 )
    return false;

  const auto deadline = _started_at + std::chrono::seconds(grace);
  return std::chrono::steady_clock::now() < deadline;
}

const std::string& longpolling_auth::channel() const
{
  static const std::string empty;
  return _opt != nullptr ? _opt->channel : empty;
}

auth_status longpolling_auth::check_session_with_grace(
  user_id_t oid,
  const std::string& sid) const
{
  if ( oid == 0 || sid.empty() )
    return auth_status::empty;

  if ( _session_store.valid(oid, sid) )
    return auth_status::ok;

  // Пара есть в store, но TTL уже истёк (ещё не вычищена expire)
  if ( _session_store.known(oid, sid) )
    return auth_status::expired;

  // По oid уже были/есть другие sid — клиент прислал неверный или устаревший sid
  if ( _session_store.oid_known(oid) )
    return auth_status::wrong_sid;

  if ( this->in_grace_period() )
    return auth_status::ok;

  // По oid в store ничего нет — comet.auth на этого пользователя не приходил
  return auth_status::no_auth;
}

auth_status longpolling_auth::check_credentials(
  user_id_t oid,
  const std::string& sid) const
{
  if ( oid == 0 && sid.empty() )
  {
    const bool allow = _opt != nullptr && _opt->allow_anonymous;
    return allow ? auth_status::ok : auth_status::anonymous_denied;
  }

  return this->check_session_with_grace(oid, sid);
}

auth_status longpolling_auth::check_agent_access(
  const longpolling& lp,
  const std::string& uuid,
  user_id_t oid,
  const std::string& sid) const
{
  user_id_t agent_oid = 0;
  if ( !lp.get_agent_oid(uuid, &agent_oid) )
    return auth_status::no_agent;

  if ( oid == 0 && sid.empty() )
  {
    if ( _opt == nullptr || !_opt->allow_anonymous )
      return auth_status::anonymous_denied;
    return agent_oid == 0 ? auth_status::ok : auth_status::oid_mismatch;
  }

  const auth_status st = this->check_session_with_grace(oid, sid);
  if ( st != auth_status::ok )
    return st;

  if ( agent_oid != 0 && agent_oid != oid )
    return auth_status::oid_mismatch;

  return auth_status::ok;
}

void longpolling_auth::expire()
{
  _session_store.expire();
}

void longpolling_auth::note_unauthorized(auth_status reason)
{
  if ( reason == auth_status::ok )
    return;

  ++_err_total;
  switch ( reason )
  {
  case auth_status::empty: ++_err_empty; break;
  case auth_status::expired: ++_err_expired; break;
  case auth_status::wrong_sid: ++_err_wrong_sid; break;
  case auth_status::no_auth: ++_err_no_auth; break;
  case auth_status::no_agent: ++_err_no_agent; break;
  case auth_status::oid_mismatch: ++_err_oid_mismatch; break;
  case auth_status::anonymous_denied: ++_err_anonymous_denied; break;
  case auth_status::ok:
  default:
    break;
  }
}

auth_error_stat longpolling_auth::take_error_stat(const longpolling& lp)
{
  auth_error_stat st;
  st.total = _err_total.exchange(0);
  st.empty = _err_empty.exchange(0);
  st.expired = _err_expired.exchange(0);
  st.wrong_sid = _err_wrong_sid.exchange(0);
  st.no_auth = _err_no_auth.exchange(0);
  st.no_agent = _err_no_agent.exchange(0);
  st.oid_mismatch = _err_oid_mismatch.exchange(0);
  st.anonymous_denied = _err_anonymous_denied.exchange(0);
  st.fail_erased = _fail_erased.exchange(0);
  st.unauthorized_agents = lp.count_agents_without_oid();
  st.sessions = _session_store.size();
  {
    std::lock_guard<std::mutex> lk(_fail_sid_oid_mutex);
    st.fail_sid_oid = _fail_sid_oid.size();
  }
  return st;
}

void longpolling_auth::fire_log(const longpolling& lp) const
{
  const size_t sessions = _session_store.size();
  const size_t unauthorized = lp.count_agents_without_oid();
  size_t fail_sid_oid = 0;
  {
    std::lock_guard<std::mutex> lk(_fail_sid_oid_mutex);
    fail_sid_oid = _fail_sid_oid.size();
  }
  const time_t grace = _opt != nullptr ? _opt->grace_period_s : 0;
  const bool is_suspend = this->suspend();

  if ( grace > 0 )
  {
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::steady_clock::now() - _started_at
    ).count();
    if ( elapsed < grace )
    {
      LONGPOLL_LOG_WARNING(
        "Auth grace: sessions=" << sessions
        << " unauthorized_agents=" << unauthorized
        << " fail_sid_oid=" << fail_sid_oid
        << (is_suspend ? " suspend=1" : "")
        << " grace_remaining_s=" << (grace - elapsed)
      );
      return;
    }
  }

  LONGPOLL_LOG_WARNING(
    "Auth: sessions=" << sessions
    << " unauthorized_agents=" << unauthorized
    << " fail_sid_oid=" << fail_sid_oid
    << (is_suspend ? " suspend=1" : "")
  );
}

void longpolling_auth::log_create_unauthorized(
  user_id_t oid,
  const std::string& sid,
  auth_status reason)
{
  static constexpr size_t MAX_SIZE = 1000000;
  size_t set_size = 0;
  {
    std::lock_guard<std::mutex> lk(_fail_sid_oid_mutex);
    if ( !_fail_sid_oid.insert(session_key{oid, sid}).second )
      return;
    if ( _fail_sid_oid.size() > MAX_SIZE )
      _fail_sid_oid.clear();
    set_size = _fail_sid_oid.size();
  }
  LONGPOLL_LOG_WARNING("create: unauthorized oid=" << oid << " sid=" << sid
                       << " reason=" << auth_status_str(reason)
                       << " fail_sid_oid=" << set_size)
}

void longpolling_auth::clear_fail_log(user_id_t oid)
{
  size_t set_size = 0;
  size_t erased = 0;
  {
    std::lock_guard<std::mutex> lk(_fail_sid_oid_mutex);
    auto it = _fail_sid_oid.lower_bound(session_key{oid, std::string()});
    while ( it != _fail_sid_oid.end() && it->oid == oid )
    {
      it = _fail_sid_oid.erase(it);
      ++erased;
    }
    set_size = _fail_sid_oid.size();
  }
  if ( erased > 0 )
  {
    _fail_erased.fetch_add(erased);
    LONGPOLL_LOG_MESSAGE("auth ok, clear_fail_log oid=" << oid
                         << " erased=" << erased
                         << " fail_sid_oid=" << set_size)
  }
}

void longpolling_auth::process_auth_message(const message& m)
{
  if ( m.content == nullptr )
    return;

  auth_session session;
  auth_session_json::serializer ser;
  ser(session, m.content->begin(), m.content->end(), nullptr);

  if ( session.oid == 0 || session.sid.empty() )
    return;

  time_t ttl = _opt != nullptr ? _opt->ttl_s : 0;
  if ( m.lifetime > 0 )
    ttl = m.lifetime;

  if ( m.action == actions::remove )
  {
    _session_store.revoke(session.oid, session.sid);
  }
  else
  {
    const size_t max_per_oid = _opt != nullptr ? _opt->max_sessions_per_oid : 0;
    _session_store.upsert(session.oid, session.sid, ttl, max_per_oid);
    this->clear_fail_log(session.oid);
  }
}

bool longpolling_auth::try_handle_hub_message(const topic& m)
{
  if ( !this->configured() || m.channel != this->channel() )
    return false;

  this->process_auth_message(m);
  return true;
}

void longpolling_auth::ensure_subscribed(longpolling& lp) const
{
  if ( !this->configured() )
    return;

  lp.ensure_subscribed(this->channel());
  LONGPOLL_LOG_MESSAGE("Auth channel subscribed: " << this->channel())
}

void longpolling_auth::unsubscribe(longpolling& lp) const
{
  if ( !this->configured() )
    return;

  lp.unsubscribe_channel(this->channel());
}

}}
