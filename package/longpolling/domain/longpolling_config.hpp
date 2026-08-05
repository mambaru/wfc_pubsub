//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <string>
#include <longpolling/longpolling_options.hpp>
#include <longpolling/auth/auth_options.hpp>
#include <longpolling/publish_options.hpp>
namespace wfc{ namespace pubsub{

struct longpolling_config: longpolling_options
{
  std::string target;
  time_t fire_timer_ms = 1000;
  time_t fire_log_s = 10;
  time_t ping_timer_ms = 1000;

  // Ограничение числа каналов на один запрос subscribe к хабу pubsub
  size_t subscribe_batch = 100;

  // Максимум одновременных запросов subscribe к хабу, ожидающих ответа
  size_t subscribe_inflight_max = 32;

  // Максимум одновременных запросов describe к хабу, ожидающих ответа
  size_t describe_inflight_max = 32;

  auth_options auth;
  publish_options publish;
};

}}
