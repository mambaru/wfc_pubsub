//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2023
//
// Copyright: See COPYING file that comes with this distribution
//


#include "http_multiton.hpp"
#include "http_domain.hpp"
#include "http_config_json.hpp"

#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{

namespace
{
  WFC_NAME2(object_name, "http")

  class impl: public ::wfc::multiton<
    object_name,
    wfc::instance<http_domain>,
    http_config_json
  >
  {};
}

http_multiton::http_multiton()
  : component( std::make_shared<impl>() )
{
}

}
