//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2023
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <longpolling/agent_options.hpp>
#include <longpolling/channel_params.hpp>
#include <longpolling/types.hpp>
#include <functional>
#include <memory>

namespace wfc{ namespace pubsub{

namespace request{

  struct create: agent_options
  {
    channel_params::channel_list_t channels;
    typedef std::unique_ptr<create> ptr;
  };
}

namespace response{

  struct create
  {
    std::string uuid;
    typedef std::unique_ptr<create> ptr;
    typedef std::function<void(ptr)> callback;
  };

}

namespace depr{
  namespace request{
    using create = ::wfc::pubsub::request::create;
  }
  namespace response{
    using create = ::wfc::pubsub::response::create;
  }
}

/*namespace depr{

namespace request{

  struct create
  {
    std::string uuid;
    typedef std::unique_ptr<create> ptr;
  };

}

}*/

}}

