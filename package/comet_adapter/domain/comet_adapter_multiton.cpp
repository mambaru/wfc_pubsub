//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//


#include "comet_adapter_multiton.hpp"
#include "comet_adapter_domain.hpp"
#include "comet_adapter_config_json.hpp"

#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace comet_adapter{

namespace
{
  WFC_NAME2(object_name, "comet-adapter")

  class impl: public ::wfc::multiton<
    object_name,
    wfc::instance<comet_adapter_domain>,
    comet_adapter_config_json
  >
  {};
}

comet_adapter_multiton::comet_adapter_multiton()
  : component( std::make_shared<impl>() )
{
}

}}
