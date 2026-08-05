

//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2023
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once

#include <longpolling/agent_params.hpp>
#include <pubsub/api/publish.hpp>
#include <functional>
#include <memory>

namespace wfc{ namespace pubsub{

namespace request{

  struct longpoll: agent_params
  {
    typedef std::unique_ptr<longpoll> ptr;
  };
}

namespace response{

  struct longpoll
  {
    topic::topic_list_t messages;
    typedef std::unique_ptr<longpoll> ptr;
    typedef std::function<void(ptr)> callback;
  };

}

}}

