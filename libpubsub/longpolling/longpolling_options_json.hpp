#pragma once

#include <longpolling/longpolling_options.hpp>
#include <longpolling/agent_map_options_json.hpp>
#include <wjson/wjson.hpp>

namespace wfc{ namespace pubsub{

struct longpolling_options_json
{
  JSON_NAME(describe_delay_s)
  JSON_NAME(inflight_timeout_s)
  JSON_NAME(describe_suspend)

  typedef wjson::object<
    longpolling_options,
    wjson::member_list<
      wjson::base<agent_map_options_json>,
      wjson::member< n_describe_delay_s, longpolling_options, time_t, &longpolling_options::describe_delay_s, wjson::time_interval_s<> >,
      wjson::member< n_inflight_timeout_s, longpolling_options, time_t, &longpolling_options::inflight_timeout_s, wjson::time_interval_s<> >,
      wjson::member< n_describe_suspend, longpolling_options, bool, &longpolling_options::describe_suspend >
    >,
    wjson::strict_mode
  > meta;

  typedef meta::serializer serializer;
  typedef meta::target target;
  typedef meta::member_list member_list;
};

}}
