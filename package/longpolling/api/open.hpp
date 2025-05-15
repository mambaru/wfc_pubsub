

//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2023
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <longpolling/agent_params.hpp>
#include <longpolling/channel_params.hpp>
#include <longpolling/types.hpp>
#include <functional>
#include <memory>

namespace wfc{ namespace pubsub{

namespace request{

  struct open: agent_params
  {
    channel_params::channel_list_t channels;
    typedef std::unique_ptr<open> ptr;
  };
}

namespace response{

  struct open
  {
    typedef std::unique_ptr<open> ptr;
    typedef std::function<void(ptr)> callback;
  };

}

}}


