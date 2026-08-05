//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <cstddef>

namespace wfc{ namespace pubsub{

/**
 * @brief Счётчики отказов авторизации за период (сбрасываются при take)
 *        и текущие gauge-значения.
 */
struct auth_error_stat
{
  // Дельты за период с прошлого take
  size_t total = 0;
  size_t empty = 0;
  size_t expired = 0;
  size_t wrong_sid = 0;
  size_t no_auth = 0;
  size_t no_agent = 0;
  size_t oid_mismatch = 0;
  size_t anonymous_denied = 0;
  /// Сколько (oid,sid) снято через clear_fail_log за период
  size_t fail_erased = 0;

  // Текущее состояние
  size_t fail_sid_oid = 0;
  size_t unauthorized_agents = 0;
  size_t sessions = 0;
};

}}
