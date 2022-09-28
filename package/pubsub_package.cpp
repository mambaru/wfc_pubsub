//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "pubsub_build_info.h"
#include "pubsub_package.hpp"
#include "pubsub/pubsub_module.hpp"
#include "comet_adapter/comet_adapter_module.hpp"
#include <wfc/module/module_list.hpp>

namespace wfc{

namespace
{
  class impl: public wfc::module_list<
    pubsub_build_info,
    pubsub::pubsub_module,
    comet_adapter::comet_adapter_module
  >
  {};
}

pubsub_package::pubsub_package()
  : wfc::package( std::make_shared<impl>() )
{
}

}
