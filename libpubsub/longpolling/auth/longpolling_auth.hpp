//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <longpolling/auth/auth_status.hpp>
#include <longpolling/auth/auth_error_stat.hpp>
#include <longpolling/auth/auth_options.hpp>
#include <longpolling/auth/session_store.hpp>
#include <message_queue/topic.hpp>
#include <atomic>
#include <chrono>
#include <mutex>
#include <set>
#include <string>

namespace wfc{ namespace pubsub{

class longpolling;

/**
 * @brief Авторизация longpolling: session store, проверки oid/sid, канал comet.auth
 */
class longpolling_auth
{
public:
  void bind(const auth_options& opt);
  void start();

  bool configured() const;
  bool enabled() const;
  bool suspend() const;
  bool in_grace_period() const;

  auth_status check_session_with_grace(user_id_t oid, const std::string& sid) const;
  auth_status check_credentials(user_id_t oid, const std::string& sid) const;
  auth_status check_agent_access(
    const longpolling& lp,
    const std::string& uuid,
    user_id_t oid,
    const std::string& sid) const;

  void expire();
  void fire_log(const longpolling& lp) const;

  /// Учесть отказ авторизации (для fire meters)
  void note_unauthorized(auth_status reason);
  /// Забрать дельты счётчиков и текущие gauge; сбросить дельты
  auth_error_stat take_error_stat(const longpolling& lp);

  void log_create_unauthorized(user_id_t oid, const std::string& sid, auth_status reason);
  /// Убрать из fail-лога все записи для oid
  void clear_fail_log(user_id_t oid);

  void process_auth_message(const message& m);
  /// @return true если сообщение обработано как auth (не пушить в агенты)
  bool try_handle_hub_message(const topic& m);

  void ensure_subscribed(longpolling& lp) const;
  void unsubscribe(longpolling& lp) const;

  const std::string& channel() const;

private:
  const auth_options* _opt = nullptr;
  session_store _session_store;
  std::chrono::steady_clock::time_point _started_at{};
  mutable std::mutex _fail_sid_oid_mutex;
  std::set<session_key> _fail_sid_oid;

  std::atomic<size_t> _err_total{0};
  std::atomic<size_t> _err_empty{0};
  std::atomic<size_t> _err_expired{0};
  std::atomic<size_t> _err_wrong_sid{0};
  std::atomic<size_t> _err_no_auth{0};
  std::atomic<size_t> _err_no_agent{0};
  std::atomic<size_t> _err_oid_mismatch{0};
  std::atomic<size_t> _err_anonymous_denied{0};
  std::atomic<size_t> _fail_erased{0};
};

}}
