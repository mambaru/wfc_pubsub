
//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "pubsub_module.hpp"
#include "domain/pubsub_multiton.hpp"
#include "service/pubsub_service_multiton.hpp"
#include "gateway/pubsub_gateway_multiton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace pubsub{

namespace
{
  WFC_NAME2(pubsub_module_name, "pubsub-module")

  class impl:
    public wfc::component_list<
      pubsub_module_name,
      pubsub_multiton,
      pubsub_service_multiton,
      pubsub_gateway_multiton
    >
  {
    virtual std::string description() const override
    {
      return "pubsub module description";
    }
  };
}

pubsub_module::pubsub_module()
  : module( std::make_shared<impl>() )
{
}

}}
