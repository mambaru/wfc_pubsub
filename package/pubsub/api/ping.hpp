//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <functional>
#include <vector>
#include <memory>

namespace wfc{ namespace pubsub{

namespace request{

  struct ping
  {
    typedef std::unique_ptr<ping> ptr;
  };
}

namespace response{

  struct ping
  {
    std::string uuid;
    typedef std::unique_ptr<ping> ptr;
    typedef std::function<void(ptr)> callback;
  };

}

}}


