//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2023
//
// Copyright: See COPYING file that comes with this distribution
//

#include "pubsub_gateway_multiton.hpp"
#include "pubsub_gateway.hpp"

#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace pubsub{

namespace {

  WFC_NAME2(component_name, "pubsub-gateway")
  class impl
    : public jsonrpc::gateway_multiton< component_name, pubsub_gateway_method_list, pubsub_interface>
  {
  public:
    virtual std::string interface_name() const override
    {
      return std::string("wfc::pubsub::ipubsub");
    }

    virtual std::string description() const override
    {
      return "Gateway for pubsub";
    }
  };
}

pubsub_gateway_multiton::pubsub_gateway_multiton()
  : wfc::component( std::make_shared<impl>() )
{
}

}}
