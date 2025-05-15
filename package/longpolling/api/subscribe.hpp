

//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2023
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once

#include <message_queue/topic.hpp>
#include <pubsub/api/publish.hpp>
#include <functional>
#include <memory>

namespace wfc{ namespace pubsub{ namespace depr{

namespace request{

  struct subscribe
  {
    typedef topic::topic_list_t message_list;
    typedef std::unique_ptr<message_list> ptr;
  };

}

namespace response{

  struct subscribe
  {
    typedef topic::topic_list_t message_list;
    typedef std::unique_ptr<message_list> ptr;
    typedef std::function<void(ptr)> callback;
  };

}


}}}


