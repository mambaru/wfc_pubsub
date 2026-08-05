//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2013, 2014, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <message_queue/message.hpp>
#include <functional>
#include <deque>

namespace wfc{ namespace pubsub{

typedef std::function<void(const std::string&, const message&)> publish_fun_t;

struct subscribe_params
{
  std::string channel;
  cursor_t cursor = 1;
  size_t limit = 0;
  publish_fun_t handler;
  typedef std::deque<subscribe_params> params_list_t;
};



}}

