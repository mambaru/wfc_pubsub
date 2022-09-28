//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once

#include "pubsub_config.hpp"
#include <wfc/json.hpp>
#include <pubsub/rocksdb/rocksdb_options_json.hpp>
#include <pubsub/pubsub_options_json.hpp>

namespace wfc{ namespace pubsub{

struct pubsub_config_json
{
  JSON_NAME(debug_reset)

  typedef wjson::object<
    pubsub_config,
    wjson::member_list<
      wjson::base<pubsub_options_json>,
      wjson::base<rocksdb_options_json>,
      wjson::member<n_debug_reset, pubsub_config, bool, &pubsub_config::debug_reset>
    >,
    wjson::strict_mode
  > type;

  typedef type::serializer serializer;
  typedef type::target target;
  typedef type::member_list member_list;
};

}}
