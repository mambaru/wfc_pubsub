#pragma once
#include <cstdint>

namespace wfc{ namespace pubsub{

struct pubsub_options
{
  bool persistent_default = false;
  bool rocksdb_disabled = false;
  bool cache_disabled = false;

  time_t default_lifetime = 60;
  size_t default_limit = 10;
  std::size_t hash_size = 64;
};

}}
