
//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "comet_adapter_module.hpp"
#include "domain/comet_adapter_multiton.hpp"
#include "service/comet_adapter_service_multiton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace comet_adapter{

namespace
{
  WFC_NAME2(comet_adapter_module_name, "comet_adapter-module")

  class impl:
    public wfc::component_list<
      comet_adapter_module_name,
      comet_adapter_multiton,
      comet_adapter_service_multiton
    >
  {
    virtual std::string description() const override
    {
      return "comet_adapter module description";
    }
  };
}

comet_adapter_module::comet_adapter_module()
  : module( std::make_shared<impl>() )
{
}

}}
