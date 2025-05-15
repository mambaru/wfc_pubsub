//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <string>
#include <longpolling/longpolling_options.hpp>
namespace wfc{ namespace pubsub{

struct longpolling_config: longpolling_options
{
  std::string target;
  time_t fire_timer_ms = 1000;
  time_t fire_log_s = 10;
  time_t ping_timer_ms = 1000;

  // Ограничение числа каналов на один запрос subscribe к хабу pubsub
  size_t subscribe_batch = 100;
};

}}
