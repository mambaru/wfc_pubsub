//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "comet_adapter_service_multiton.hpp"
#include "comet_adapter_service.hpp"

#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace comet_adapter{

namespace
{
  WFC_NAME2(service_name, "comet-adapter-service")

  class impl
    : public ::wfc::jsonrpc::service_multiton< service_name, service_method_list, comet_adapter_interface>
  {};
}

comet_adapter_service_multiton::comet_adapter_service_multiton()
  : wfc::component( std::make_shared<impl>() )
{
}

}}
