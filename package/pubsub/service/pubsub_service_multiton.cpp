//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "pubsub_service_multiton.hpp"
#include "pubsub_service.hpp"

#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace pubsub{

namespace
{
  WFC_NAME2(service_name, "pubsub-service")

  class impl
    : public ::wfc::jsonrpc::service_multiton< service_name, service_method_list, pubsub_interface>
  {};
}

pubsub_service_multiton::pubsub_service_multiton()
  : wfc::component( std::make_shared<impl>() )
{
}

}}
