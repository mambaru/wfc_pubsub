//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once

#include <pubsub/pubsub_options.hpp>
#include <wjson/json.hpp>
#include <pubsub/rocksdb/rocksdb_options_json.hpp>

namespace wfc{ namespace pubsub{

struct pubsub_options_json
{
  JSON_NAME(hash_size)
  JSON_NAME(persistent_default)
  JSON_NAME(rocksdb_disabled)
  JSON_NAME(cache_disabled)
  JSON_NAME(default_lifetime_s)
  JSON_NAME(default_limit)

  typedef wjson::object<
    pubsub_options,
    wjson::member_list<
      wjson::member<n_persistent_default, pubsub_options, bool, &pubsub_options::persistent_default>,
      wjson::member<n_rocksdb_disabled, pubsub_options, bool, &pubsub_options::rocksdb_disabled>,
      wjson::member<n_cache_disabled, pubsub_options, bool, &pubsub_options::cache_disabled>,
      wjson::member<n_default_lifetime_s, pubsub_options, time_t, &pubsub_options::default_lifetime, wjson::time_interval_s<time_t> >,
      wjson::member<n_default_limit, pubsub_options, size_t, &pubsub_options::default_limit>,
      wjson::member<n_hash_size, pubsub_options, size_t, &pubsub_options::hash_size>
    >,
    wjson::strict_mode
  > type;

  typedef type::serializer serializer;
  typedef type::target target;
  typedef type::member_list member_list;
};

}}

