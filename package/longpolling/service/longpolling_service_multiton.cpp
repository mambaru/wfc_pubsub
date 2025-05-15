//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "longpolling_service_multiton.hpp"
#include "longpolling_service.hpp"

#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace pubsub{

namespace
{
  WFC_NAME2(service_name, "longpolling-service")

  class impl
    : public ::wfc::jsonrpc::service_multiton< service_name, longpolling_service_method_list, depr::comet_subscriber>
  {};
}

longpolling_service_multiton::longpolling_service_multiton()
  : wfc::component( std::make_shared<impl>() )
{
}

}}
