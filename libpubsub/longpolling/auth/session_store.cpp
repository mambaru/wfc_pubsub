#include "session_store.hpp"

namespace wfc{ namespace pubsub{

void session_store::erase_(const session_key& key, time_t expire_at)
{
  _sessions.erase(key);
  _by_expire.erase( std::make_pair(expire_at, key) );
  _by_oid_expire.erase( std::make_tuple(key.oid, expire_at, key.sid) );
}

void session_store::trim_oid_(user_id_t oid, size_t max_per_oid)
{
  if ( max_per_oid == 0 )
    return;

  auto first = _by_oid_expire.lower_bound( std::make_tuple(oid, time_t(0), std::string()) );
  size_t count = 0;
  for ( auto itr = first; itr != _by_oid_expire.end() && std::get<0>(*itr) == oid; ++itr )
    ++count;

  while ( count > max_per_oid )
  {
    auto itr = _by_oid_expire.lower_bound( std::make_tuple(oid, time_t(0), std::string()) );
    if ( itr == _by_oid_expire.end() || std::get<0>(*itr) != oid )
      break;

    session_key key{ std::get<0>(*itr), std::get<2>(*itr) };
    this->erase_(key, std::get<1>(*itr));
    --count;
  }
}

void session_store::upsert(user_id_t oid, const std::string& sid, time_t ttl_s, size_t max_per_oid)
{
  if ( oid == 0 || sid.empty() || ttl_s <= 0 )
    return;

  std::lock_guard<mutex_type> lk(_mutex);
  session_key key{oid, sid};
  const time_t expire_at = time(nullptr) + ttl_s;

  auto itr = _sessions.find(key);
  if ( itr != _sessions.end() )
  {
    _by_expire.erase( std::make_pair(itr->second, key) );
    _by_oid_expire.erase( std::make_tuple(oid, itr->second, sid) );
    itr->second = expire_at;
  }
  else
  {
    _sessions[key] = expire_at;
  }

  _by_expire.insert( std::make_pair(expire_at, key) );
  _by_oid_expire.insert( std::make_tuple(oid, expire_at, sid) );

  this->trim_oid_(oid, max_per_oid);
}

void session_store::revoke(user_id_t oid, const std::string& sid)
{
  std::lock_guard<mutex_type> lk(_mutex);
  session_key key{oid, sid};
  auto itr = _sessions.find(key);
  if ( itr == _sessions.end() )
    return;

  this->erase_(key, itr->second);
}

bool session_store::valid(user_id_t oid, const std::string& sid) const
{
  if ( oid == 0 || sid.empty() )
    return false;

  std::lock_guard<mutex_type> lk(_mutex);
  auto itr = _sessions.find(session_key{oid, sid});
  if ( itr == _sessions.end() )
    return false;
  return itr->second > time(nullptr);
}

bool session_store::known(user_id_t oid, const std::string& sid) const
{
  if ( oid == 0 || sid.empty() )
    return false;

  std::lock_guard<mutex_type> lk(_mutex);
  return _sessions.find(session_key{oid, sid}) != _sessions.end();
}

bool session_store::oid_known(user_id_t oid) const
{
  if ( oid == 0 )
    return false;

  std::lock_guard<mutex_type> lk(_mutex);
  auto itr = _sessions.lower_bound(session_key{oid, std::string()});
  return itr != _sessions.end() && itr->first.oid == oid;
}

void session_store::expire()
{
  const time_t now = time(nullptr);
  std::lock_guard<mutex_type> lk(_mutex);
  while ( !_by_expire.empty() && _by_expire.begin()->first <= now )
  {
    const time_t expire_at = _by_expire.begin()->first;
    const session_key key = _by_expire.begin()->second;
    this->erase_(key, expire_at);
  }
}

size_t session_store::size() const
{
  std::lock_guard<mutex_type> lk(_mutex);
  return _sessions.size();
}

}}
