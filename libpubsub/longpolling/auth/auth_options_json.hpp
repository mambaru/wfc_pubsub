#pragma once

#include <longpolling/auth/auth_options.hpp>
#include <wjson/wjson.hpp>

namespace wfc{ namespace pubsub{

struct auth_options_json
{
  JSON_NAME(disabled)
  JSON_NAME(suspend)
  JSON_NAME(channel)
  JSON_NAME(ttl_s)
  JSON_NAME(max_sessions_per_oid)
  JSON_NAME(allow_anonymous)
  JSON_NAME(grace_period_s)

  typedef wjson::object<
    auth_options,
    wjson::member_list<
      wjson::member<n_disabled, auth_options, bool, &auth_options::disabled>,
      wjson::member<n_suspend, auth_options, bool, &auth_options::suspend>,
      wjson::member<n_channel, auth_options, std::string, &auth_options::channel>,
      wjson::member<n_ttl_s, auth_options, time_t, &auth_options::ttl_s, wjson::time_interval_s<> >,
      wjson::member<n_max_sessions_per_oid, auth_options, size_t, &auth_options::max_sessions_per_oid>,
      wjson::member<n_allow_anonymous, auth_options, bool, &auth_options::allow_anonymous>,
      wjson::member<n_grace_period_s, auth_options, time_t, &auth_options::grace_period_s, wjson::time_interval_s<> >
    >,
    wjson::strict_mode
  > meta;

  typedef meta::serializer serializer;
  typedef meta::target target;
  typedef meta::member_list member_list;
};

}}
