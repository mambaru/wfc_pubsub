
//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2023
//
// Copyright: See COPYING file that comes with this distribution
//

#include "http_module.hpp"

#include "domain/http_multiton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

namespace wfc{

namespace
{
  WFC_NAME2(module_name, "http-module")

  class impl:
    public wfc::component_list<
      module_name,
      http_multiton
    >
  {
    virtual std::string description() const override
    {
      return "http module description";
    }
  };
}

http_module::http_module()
  : module( std::make_shared<impl>() )
{
}

}

