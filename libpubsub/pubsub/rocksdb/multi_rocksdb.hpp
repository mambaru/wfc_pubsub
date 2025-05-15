//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <pubsub/rocksdb/rocksdb_options.hpp>
#include <pubsub/rocksdb/rocksdb_factory.hpp>
#include <message_queue/message.hpp>
#include <memory>
#include <vector>
#include <string>
#include <ctime>
#include <mutex>
#include <map>
#include <set>

namespace wfc{ namespace pubsub{

class rocksdb;

class multi_rocksdb
{
  typedef std::shared_ptr<rocksdb> rocksdb_ptr;
  typedef std::shared_ptr<rocksdb_factory> factory_ptr;
  typedef std::map<time_t, rocksdb_ptr> rocksdb_map;
  typedef std::set<std::string> channel_set;
public:
  typedef std::vector<message> message_list_t;

  virtual ~multi_rocksdb();
  multi_rocksdb();
  bool configure( bool channels_cache,  const rocksdb_options& opt);
  void close();
  bool push( const std::string& channel, const message& m );
  bool get_messages( message_list_t* ml, const std::string& channel, cursor_t cursor, size_t limit);
  bool has( const std::string& channel ) const;
private:
  rocksdb_ptr get_db_(time_t ttl) const;
  void close_db_(time_t ttl) ;
private:
  typedef std::recursive_mutex mutex_type;
  mutable mutex_type _mutex;
  rocksdb_map _rocksdb_map;
  channel_set _channels;
  factory_ptr _factory;
  std::atomic_bool _channels_cache = false;

private:
  typedef ::rocksdb::ColumnFamilyDescriptor CFD;
  typedef std::vector<CFD> CFD_list;
  ::rocksdb::Options _options;
  CFD_list _cdf;
  rocksdb_options _config;

};

}}
