//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once

#include "longpolling_config.hpp"
#include <wfc/json.hpp>

namespace wfc{ namespace pubsub{

struct agent_map_options_json
{
  JSON_NAME(hash_size)

  typedef wjson::object<
    agent_map_options,
    wjson::member_list<
      wjson::member< n_hash_size, agent_map_options, size_t, &agent_map_options::hash_size >
    >,
    wjson::strict_mode
  > meta;

  typedef meta::serializer serializer;
  typedef meta::target target;
  typedef meta::member_list member_list;
};


struct longpolling_options_json
{
  JSON_NAME(describe_delay_s)
  JSON_NAME(lost_delay_s)
  JSON_NAME(describe_suspend)

  typedef wjson::object<
    longpolling_options,
    wjson::member_list<
      wjson::base<agent_map_options_json>,
      wjson::member< n_describe_delay_s, longpolling_options, time_t, &longpolling_options::describe_delay_s, wjson::time_interval_s<> >,
      wjson::member< n_lost_delay_s, longpolling_options, time_t, &longpolling_options::lost_delay_s, wjson::time_interval_s<> >,
      wjson::member< n_describe_suspend, longpolling_options, bool, &longpolling_options::describe_suspend >
    >,
    wjson::strict_mode
  > meta;

  typedef meta::serializer serializer;
  typedef meta::target target;
  typedef meta::member_list member_list;
};


struct longpolling_config_json
{
  JSON_NAME(target)
  JSON_NAME(fire_timer_ms)
  JSON_NAME(fire_log_s)
  JSON_NAME(ping_timer_ms)
  JSON_NAME(subscribe_batch)

  typedef wjson::object<
    longpolling_config,
    wjson::member_list<
      wjson::member< n_target, longpolling_config, std::string, &longpolling_config::target>,
      wjson::member< n_fire_timer_ms, longpolling_config, time_t, &longpolling_config::fire_timer_ms, wjson::time_interval_ms<> >,
      wjson::member< n_fire_log_s, longpolling_config, time_t, &longpolling_config::fire_log_s, wjson::time_interval_s<> >,
      wjson::member< n_ping_timer_ms, longpolling_config, time_t, &longpolling_config::ping_timer_ms, wjson::time_interval_ms<> >,
      wjson::member< n_subscribe_batch, longpolling_config, size_t, &longpolling_config::subscribe_batch >,
      wjson::base<longpolling_options_json>
    >,
    wjson::strict_mode
  > meta;

  typedef meta::serializer serializer;
  typedef meta::target target;
  typedef meta::member_list member_list;
};

}}
