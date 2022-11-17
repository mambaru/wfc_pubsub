//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <string>
#include <pubsub/pubsub_options.hpp>
#include <pubsub/rocksdb/rocksdb_options.hpp>

namespace wfc{ namespace pubsub{

struct pubsub_config
  : pubsub_options
  , rocksdb_options
{
  time_t control_s = 60;
  bool debug_reset = false;
};

}}
