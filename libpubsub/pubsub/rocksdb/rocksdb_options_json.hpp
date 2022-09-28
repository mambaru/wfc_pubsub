#pragma once

#include <pubsub/rocksdb/rocksdb_options.hpp>
#include <wjson/wjson.hpp>

namespace wfc{ namespace pubsub{

struct rocksdb_options_json
{
  JSON_NAME(path)
  JSON_NAME(wal_path)
  JSON_NAME(ini)
  JSON_NAME(slave)
  JSON_NAME(ttl)

  typedef wjson::object<
    rocksdb_options,
    wjson::member_list<
      wjson::member<n_path, rocksdb_options, std::string, &rocksdb_options::path>,
      wjson::member<n_ini,  rocksdb_options, std::string,    &rocksdb_options::ini>,
      wjson::member<n_wal_path, rocksdb_options, std::string, &rocksdb_options::wal_path>,
      wjson::member<n_ttl,  rocksdb_options, std::vector<time_t>, &rocksdb_options::ttl,
                    wjson::vector_of< wjson::time_interval_s<time_t> > >
    >,
    wjson::strict_mode
  > meta;

  typedef meta::target target;
  typedef meta::serializer serializer;
  typedef meta::member_list member_list;
};

}}

