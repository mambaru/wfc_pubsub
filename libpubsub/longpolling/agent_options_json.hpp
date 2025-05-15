
//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2013, 2014, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <longpolling/agent_options.hpp>
#include <wjson/wjson.hpp>

namespace wfc{ namespace pubsub{

struct agent_options_json
{
  JSON_NAME(uuid)
  JSON_NAME(key)
  JSON_NAME(lifetime)
  JSON_NAME(timeout)
  JSON_NAME(limit)
  JSON_NAME(prio_limit)

  typedef wjson::object<
      agent_options,
      wjson::member_list<
        wjson::member<n_uuid, agent_options, std::string, &agent_options::uuid>,
        wjson::member<n_key, agent_options, key_t, &agent_options::key>,
        wjson::member<n_lifetime, agent_options, time_t, &agent_options::agent_lifetime>,
        wjson::member<n_timeout, agent_options, time_t, &agent_options::longpoll_timeout>,
        wjson::member<n_limit, agent_options, size_t, &agent_options::longpoll_limit>,
        wjson::member<n_prio_limit, agent_options, size_t, &agent_options::prio_limit>
      >
  > meta;

  typedef meta::target target;
  typedef meta::member_list member_list;
  typedef meta::serializer serializer;
};

}}
