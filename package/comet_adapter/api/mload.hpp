
//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <comet_adapter/api/status.hpp>
#include <comet_adapter/api/load.hpp>
#include <pubsub/api/subscribe_params.hpp>
#include <pubsub/api/message.hpp>
#include <functional>
#include <memory>

namespace wfc{ namespace comet_adapter{

namespace request{

  struct mload
  {
    typedef std::vector<pubsub::subscribe_params> load_list_t;
    typedef std::unique_ptr<load_list_t> ptr;
  };
}

namespace response{

  struct mload
  {
    typedef std::vector<response::load> load_list_t;
    typedef std::unique_ptr<load_list_t> ptr;
    typedef std::function<void(ptr)> callback;
  };

}

}}

