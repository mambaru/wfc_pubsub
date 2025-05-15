
//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "longpolling_module.hpp"
#include "domain/longpolling_multiton.hpp"
#include "service/longpolling_service_multiton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace pubsub{

namespace
{
  WFC_NAME2(longpolling_module_name, "longpolling-module")

  class impl:
    public wfc::component_list<
      longpolling_module_name,
      longpolling_multiton,
      longpolling_service_multiton
    >
  {
    virtual std::string description() const override
    {
      return "longpolling module description";
    }
  };
}

longpolling_module::longpolling_module()
  : module( std::make_shared<impl>() )
{
}

}}
