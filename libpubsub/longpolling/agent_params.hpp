#pragma once

#include <string>
#include <message_queue/types.hpp>

namespace wfc{ namespace pubsub{

struct agent_params
{
  std::string uuid;
  key_t key = 0;
  bool confirm = true;

  // Идентификатор пользователя и сессии (авторизация)
  user_id_t oid = 0;
  std::string sid;

  agent_params() = default;
  explicit agent_params(std::string uuid_)
    : uuid(std::move(uuid_))
  {}
  agent_params(std::string uuid_, key_t key_, bool confirm_)
    : uuid(std::move(uuid_))
    , key(key_)
    , confirm(confirm_)
  {}
};

}}



