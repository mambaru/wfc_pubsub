//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <memory>
#include <functional>
#include <pubsub/api/message.hpp>
namespace wfc{ namespace pubsub{

namespace request{

  struct publish
  {
    struct topic:message
    {
      std::string channel;
    };
    typedef std::vector<topic> message_list_t;

    message_list_t messages;
    typedef std::unique_ptr<publish> ptr;
  };
}

namespace response{

  struct publish
  {
    typedef std::unique_ptr<publish> ptr;
    typedef std::function<void(ptr)> callback;
  };

}

}}
