//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2013, 2014, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <pubsub/api/message.hpp>
#include <functional>

namespace wfc{ namespace pubsub{

struct subscribe_params
{
  typedef std::function<void(const std::string& channel, const message& msg)> handler_fun_t;

  std::string channel;
  cursor_t cursor = 1;
  size_t limit = 0;
  handler_fun_t handler;
};

}}

