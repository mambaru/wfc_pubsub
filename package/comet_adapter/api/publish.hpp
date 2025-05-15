//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <comet_adapter/api/status.hpp>
#include <message_queue/message.hpp>
#include <functional>
#include <memory>

namespace wfc{ namespace comet_adapter{

namespace request{

  struct publish
    : pubsub::message
  {
    std::string channel;
    typedef std::unique_ptr<publish> ptr;
  };
}

namespace response{

  struct publish
  {
    typedef status::type type;
    typedef std::unique_ptr<type> ptr;
    typedef std::function<void(ptr)> callback;
  };

}

}}
