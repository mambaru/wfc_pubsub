//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <pubsub/api/publish.hpp>
#include <message_queue/subscribe_params.hpp>

#include <functional>
#include <vector>
#include <memory>

namespace wfc{ namespace pubsub{

namespace request{

  struct get_messages
  {
    typedef subscribe_params::params_list_t params_list_t;
    params_list_t channels;
    typedef std::unique_ptr<get_messages> ptr;
  };
}

namespace response{

  struct get_messages
  {
    topic::topic_list_t messages;
    typedef std::unique_ptr<get_messages> ptr;
    typedef std::function<void(ptr)> callback;
  };

}

}}
