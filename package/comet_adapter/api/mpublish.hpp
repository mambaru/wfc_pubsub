//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <comet_adapter/api/status.hpp>
#include <comet_adapter/api/publish.hpp>
#include <functional>
#include <vector>
#include <memory>

namespace wfc{ namespace comet_adapter{

namespace request{

  struct mpublish
  {
    typedef std::vector<publish> type;
    typedef std::unique_ptr<type> ptr;
  };

}

namespace response{

  struct mpublish
  {
    typedef status::type type;
    typedef std::unique_ptr<type> ptr;
    typedef std::function<void(ptr)> callback;
  };

}

}}
