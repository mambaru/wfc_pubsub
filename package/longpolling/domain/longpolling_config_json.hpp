//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once

#include "longpolling_config.hpp"
#include <longpolling/auth/auth_options_json.hpp>
#include <longpolling/publish_options_json.hpp>
#include <longpolling/longpolling_options_json.hpp>
#include <wfc/json.hpp>

namespace wfc{ namespace pubsub{

struct longpolling_config_json
{
  JSON_NAME(target)
  JSON_NAME(fire_timer_ms)
  JSON_NAME(fire_log_s)
  JSON_NAME(ping_timer_ms)
  JSON_NAME(subscribe_batch)
  JSON_NAME(subscribe_inflight_max)
  JSON_NAME(describe_inflight_max)
  JSON_NAME(auth)
  JSON_NAME(publish)

  typedef wjson::object<
    longpolling_config,
    wjson::member_list<
      wjson::member< n_target, longpolling_config, std::string, &longpolling_config::target>,
      wjson::member< n_fire_timer_ms, longpolling_config, time_t, &longpolling_config::fire_timer_ms, wjson::time_interval_ms<> >,
      wjson::member< n_fire_log_s, longpolling_config, time_t, &longpolling_config::fire_log_s, wjson::time_interval_s<> >,
      wjson::member< n_ping_timer_ms, longpolling_config, time_t, &longpolling_config::ping_timer_ms, wjson::time_interval_ms<> >,
      wjson::member< n_subscribe_batch, longpolling_config, size_t, &longpolling_config::subscribe_batch >,
      wjson::member< n_subscribe_inflight_max, longpolling_config, size_t, &longpolling_config::subscribe_inflight_max >,
      wjson::member< n_describe_inflight_max, longpolling_config, size_t, &longpolling_config::describe_inflight_max >,
      wjson::member< n_auth, longpolling_config, auth_options, &longpolling_config::auth, auth_options_json >,
      wjson::member< n_publish, longpolling_config, publish_options, &longpolling_config::publish, publish_options_json >,
      wjson::base<longpolling_options_json>
    >,
    wjson::strict_mode
  > meta;

  typedef meta::serializer serializer;
  typedef meta::target target;
  typedef meta::member_list member_list;
};

}}
