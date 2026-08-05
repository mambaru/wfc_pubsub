//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <longpolling/auth/auth_error_stat.hpp>
#include <longpolling/fire_stat.hpp>
#include <wfc/statistics/meters.hpp>
#include <vector>

namespace wfc{ namespace statistics{ class statistics; }}

namespace wfc{ namespace pubsub{

/**
 * @brief Метрики fire-тика longpolling (wrtstat value meters)
 */
class longpolling_stat
{
public:
  void start(wfc::statistics::statistics& st);
  bool enabled() const;

  void write(
    const fire_stat& cur,
    size_t describe_count,
    size_t subscribe_count,
    const auth_error_stat& auth_st);

private:
  std::vector<wfc::value_meter> _meters;
};

}}
