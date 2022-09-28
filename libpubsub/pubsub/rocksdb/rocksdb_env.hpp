#pragma once

#include <rocksdb/db.h>
#include <vector>

namespace wfc{ namespace pubsub{

struct rocksdb_env
{
  typedef ::rocksdb::ColumnFamilyDescriptor CFD;
  typedef std::vector<CFD> CFD_list;
  ::rocksdb::Env* env = nullptr;
  ::rocksdb::Options options;
  CFD_list cdf;
};

}}
