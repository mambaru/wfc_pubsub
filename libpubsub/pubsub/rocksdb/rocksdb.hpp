//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <pubsub/rocksdb/rocksdb_options.hpp>
#include <pubsub/rocksdb/rocksdb_env.hpp>
#include <rocksdb/db.h>
#include <rocksdb/write_batch.h>
#include <rocksdb/utilities/db_ttl.h>
#include <message_queue/message.hpp>
#include <vector>
#include <string>
#include <ctime>

namespace wfc{ namespace pubsub{

class rocksdb
{
  typedef ::rocksdb::DBWithTTL  db_type;
  typedef ::rocksdb::WriteBatch write_batch_t;
  typedef std::shared_ptr<write_batch_t> write_batch_ptr;
  typedef std::vector<message> message_list_t;

public:
  virtual ~rocksdb();
  bool create(time_t ttl, const rocksdb_options& opt, const rocksdb_env& env);
  bool close();
  bool push( const std::string& name, const message& m );
  bool get_messages( message_list_t* ml, const std::string& name, cursor_t cursor, size_t limit);

private:
  std::shared_ptr<db_type> _db;
  std::vector< ::rocksdb::ColumnFamilyHandle*> _handles;
};

}}
