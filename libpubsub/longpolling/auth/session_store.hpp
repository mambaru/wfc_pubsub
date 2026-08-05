#pragma once

#include <string>
#include <map>
#include <set>
#include <tuple>
#include <mutex>
#include <message_queue/types.hpp>

namespace wfc{ namespace pubsub{

struct session_key
{
  user_id_t oid = 0;
  std::string sid;

  bool operator<(const session_key& o) const noexcept
  {
    if ( oid != o.oid )
      return oid < o.oid;
    return sid < o.sid;
  }
};

/**
 * @brief Локальное хранилище авторизованных пар oid/sid с TTL
 */
class session_store
{
public:
  /// @param max_per_oid максимум sid на oid; 0 — без ограничения
  void upsert(user_id_t oid, const std::string& sid, time_t ttl_s, size_t max_per_oid = 0);
  void revoke(user_id_t oid, const std::string& sid);
  bool valid(user_id_t oid, const std::string& sid) const;
  bool known(user_id_t oid, const std::string& sid) const;
  bool oid_known(user_id_t oid) const;
  void expire();
  size_t size() const;

private:
  typedef std::map<session_key, time_t> session_map_t;
  typedef std::set< std::pair<time_t, session_key> > expire_set_t;
  // Индекс по oid, затем по expire_at — для вытеснения самого старого sid
  typedef std::set< std::tuple<user_id_t, time_t, std::string> > oid_expire_set_t;
  typedef std::mutex mutex_type;

  void erase_(const session_key& key, time_t expire_at);
  void trim_oid_(user_id_t oid, size_t max_per_oid);

  mutable mutex_type _mutex;
  session_map_t _sessions;
  expire_set_t _by_expire;
  oid_expire_set_t _by_oid_expire;
};

}}
