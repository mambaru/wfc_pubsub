//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <memory>
#include <functional>
#include <message_queue/topic.hpp>
namespace wfc{ namespace pubsub{

namespace request{

  struct publish
  {
    topic::topic_list_t messages;
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
