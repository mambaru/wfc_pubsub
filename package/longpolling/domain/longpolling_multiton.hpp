//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/module/component.hpp>

namespace wfc{ namespace pubsub{

class longpolling_multiton
  : public wfc::component
{
public:
  longpolling_multiton();
};

}}
