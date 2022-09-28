//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//


#include "pubsub_multiton.hpp"
#include "pubsub_domain.hpp"
#include "pubsub_config_json.hpp"

#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace pubsub{

namespace
{
  WFC_NAME2(object_name, "pubsub")

  class impl: public ::wfc::multiton<
    object_name,
    wfc::instance<pubsub_domain>,
    pubsub_config_json
  >
  {};
}

pubsub_multiton::pubsub_multiton()
  : component( std::make_shared<impl>() )
{
}

}}
