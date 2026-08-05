//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2023
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/module/component.hpp>

namespace wfc{ namespace pubsub{

class pubsub_gateway_multiton
  : public ::wfc::component
{
public:
  pubsub_gateway_multiton();
};

}}
