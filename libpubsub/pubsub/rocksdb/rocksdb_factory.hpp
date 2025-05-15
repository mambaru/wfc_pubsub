//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <pubsub/rocksdb/rocksdb_options.hpp>
#include <pubsub/rocksdb/rocksdb_env.hpp>
#include <pubsub/rocksdb/rocksdb.hpp>
#include <mutex>

namespace wfc{ namespace pubsub{

class rocksdb_factory
{
public:
  typedef std::shared_ptr<rocksdb> rocksdb_ptr;
  virtual ~rocksdb_factory();
  bool configure(const rocksdb_options& opt);
  rocksdb_ptr create(time_t ttl) const;
private:
  typedef std::recursive_mutex mutex_type;
  mutable mutex_type _mutex;
  rocksdb_options _opt;
  rocksdb_env _env;
};

}}




