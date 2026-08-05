#pragma once

#include <string>
#include <message_queue/types.hpp>
#include <wjson/wjson.hpp>

namespace wfc{ namespace pubsub{

struct auth_session
{
  user_id_t oid = 0;
  std::string sid;
  std::string action;
};

struct auth_session_json
{
  JSON_NAME(oid)
  JSON_NAME(sid)
  JSON_NAME(action)

  typedef wjson::object<
    auth_session,
    wjson::member_list<
      wjson::member<n_oid, auth_session, user_id_t, &auth_session::oid>,
      wjson::member<n_sid, auth_session, std::string, &auth_session::sid>,
      wjson::member<n_action, auth_session, std::string, &auth_session::action>
    >
  > meta;

  typedef meta::serializer serializer;
};

}}
