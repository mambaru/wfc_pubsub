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

  struct describe
  {
    std::vector<std::string> channels;
    typedef std::unique_ptr<describe> ptr;
  };
}

namespace response{

  struct describe
  {
    typedef std::unique_ptr<describe> ptr;
    typedef std::function<void(ptr)> callback;
  };

}

}}

