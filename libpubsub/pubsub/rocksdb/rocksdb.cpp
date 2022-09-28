//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "rocksdb.hpp"
#include "../api/message_json.hpp"
#include <pubsub/logger.hpp>
#include <rocksdb/db.h>
#include "../message_queue.hpp"

namespace wfc{ namespace pubsub{

rocksdb::~rocksdb()
{

}

bool rocksdb::close()
{
  if ( _db!=nullptr )
  {
    for ( auto& handle: _handles)
      _db->DestroyColumnFamilyHandle(handle);
    _db->Close();
  }
  _db.reset();
  return true;
}

bool rocksdb::create(time_t ttl, const rocksdb_options& opt, const rocksdb_env& env)
{
  std::string suffix;
  wjson::time_interval_s<time_t>::serializer()(ttl, std::back_inserter(suffix));
  suffix.erase(suffix.begin()); // Удаляем кавычки
  suffix.pop_back();
  std::string path = opt.path + "/pubsub-"+suffix;
  PUBSUB_LOG_MESSAGE("rocksdb::DB::Open '" << path << "'" )

  ::rocksdb::DBWithTTL* db = nullptr;

  std::vector<int32_t> ttls;
  ttls.push_back( static_cast<int32_t>(ttl) );
  ::rocksdb::Status status = ::rocksdb::DBWithTTL::Open(env.options, path, env.cdf , &_handles, &db, ttls);
  _db = std::shared_ptr<db_type>(db);

  if ( !status.ok() )
  {
    PUBSUB_LOG_FATAL("rocksdb::DB::Open '" << path << "' :" << status.ToString());
  }
  return status.ok();
}

bool rocksdb::push( const std::string& name, const message& m )
{
  typedef wjson::vector_of< wjson::pointer< const message*, message_json> > message_list_json;
  std::vector< const message* > message_list;
  message_list.push_back( &m );
  std::string json_value;
  message_list_json::serializer()(message_list, std::back_inserter(json_value) );

  ::rocksdb::WriteOptions wo;
  _db->Merge(wo, name, json_value);
  return false;
}

bool rocksdb::get_messages( message_list_t* ml, const std::string& name, cursor_t cursor, size_t limit)
{
  std::string value;
  if ( !_db->Get(::rocksdb::ReadOptions(), name, &value).ok() )
    return false;

  if (value.empty())
    return false;

  typedef wjson::vector_of<message_json> message_list_json;
  message_queue mqueue;
  message_list_t message_list;
  message_list.reserve(16);

  wjson::json_error je;
  message_list_json::serializer()(message_list, value.begin(), value.end(), &je);

  if ( je )
  {
    PUBSUB_LOG_ERROR("PUBSUB rocksdb::get_messages json_error: "
        << wjson::strerror::message_trace(je, value.begin(), value.end())  )
    return false;
  }

  PUBSUB_LOG_TRACE("get_messages result: " << value)


  for ( auto& m : message_list)
  {
    mqueue.push( std::move(m) );
  }

  mqueue.remove_death( time(nullptr) );
  //size_t removed = mqueue.get_removed(true);
  //PUBSUB_LOG_TRACE("get_removed result: " << removed)
  mqueue.copy_to(cursor, limit, std::back_inserter(*ml));
  return true;
}


}}
