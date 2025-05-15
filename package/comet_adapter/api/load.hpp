
//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <comet_adapter/api/status.hpp>
#include <message_queue/subscribe_params.hpp>
#include <message_queue/message.hpp>
#include <functional>
#include <memory>

namespace wfc{ namespace comet_adapter{

namespace request{

  struct load
    : pubsub::subscribe_params
  {
    typedef std::unique_ptr<load> ptr;
  };
}

namespace response{

  struct load
  {
    typedef std::vector<pubsub::message> message_list;
    std::string channel;
    message_list messages;
    status::type load_status = status::ready;
    typedef std::unique_ptr<load> ptr;
    typedef std::function<void(ptr)> callback;
  };

}

}}
