//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <pubsub/api/publish.hpp>
#include <pubsub/api/subscribe_params.hpp>

#include <functional>
#include <vector>
#include <memory>

namespace wfc{ namespace pubsub{

namespace request{

  struct subscribe
  {
    std::vector<subscribe_params> channels;
    typedef std::unique_ptr<subscribe> ptr;
  };
}

namespace response{

  struct subscribe
  {
    request::publish::message_list_t messages;

    typedef std::unique_ptr<subscribe> ptr;
    typedef std::function<void(ptr)> callback;
    typedef std::function<void(request::publish::ptr)> handler;
  };

}

}}
