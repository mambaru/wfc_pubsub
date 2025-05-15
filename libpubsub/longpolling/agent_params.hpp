#pragma once

#include <string>
#include <message_queue/types.hpp>

namespace wfc{ namespace pubsub{

struct agent_params
{
  std::string uuid;
  key_t key = 0;
  bool confirm = true;
};

}}



