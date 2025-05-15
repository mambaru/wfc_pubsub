

//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2023
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <functional>
#include <memory>

namespace wfc{ namespace pubsub{

namespace request{

  struct close
  {
    std::string uuid;
    bool confirm = true;
    std::vector<std::string> channels;
    typedef std::unique_ptr<close> ptr;
  };
}

namespace response{

  struct close
  {
    typedef std::unique_ptr<close> ptr;
    typedef std::function<void(ptr)> callback;
  };

}

}}


