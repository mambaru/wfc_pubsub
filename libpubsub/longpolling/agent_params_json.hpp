

//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2013, 2014, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <longpolling/agent_params.hpp>
#include <wjson/wjson.hpp>

namespace wfc{ namespace pubsub{

struct agent_params_json
{
  JSON_NAME(uuid)
  JSON_NAME(key)
  JSON_NAME(confirm)
  JSON_NAME(oid)
  JSON_NAME(sid)

  typedef wjson::object<
      agent_params,
      wjson::member_list<
        wjson::member<n_uuid, agent_params, std::string, &agent_params::uuid>,
        wjson::member<n_key, agent_params, key_t, &agent_params::key>,
        wjson::member<n_confirm, agent_params, bool, &agent_params::confirm>,
        wjson::member<n_oid, agent_params, user_id_t, &agent_params::oid>,
        wjson::member<n_sid, agent_params, std::string, &agent_params::sid>
      >
  > meta;

  typedef meta::target target;
  typedef meta::member_list member_list;
  typedef meta::serializer serializer;
};

}}
