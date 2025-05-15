//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//


#include "longpolling_multiton.hpp"
#include "longpolling_domain.hpp"
#include "longpolling_config_json.hpp"

#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace pubsub{

namespace
{
  WFC_NAME2(object_name, "longpolling")

  class impl: public ::wfc::multiton<
    object_name,
    wfc::instance<longpolling_domain>,
    longpolling_config_json,
    component_features::Defaults,
    defstat_json
  >
  {};
}

longpolling_multiton::longpolling_multiton()
  : component( std::make_shared<impl>() )
{
}

}}
