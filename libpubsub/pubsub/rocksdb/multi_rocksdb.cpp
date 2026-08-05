//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "multi_rocksdb.hpp"
#include <set>
#include <utility>
#include <algorithm>
#include "../logger.hpp"

namespace wfc{ namespace pubsub{

multi_rocksdb::~multi_rocksdb()
{

}

void multi_rocksdb::close()
{
  rocksdb_map closing;
  {
    std::lock_guard<mutex_type> lk(_mutex);
    closing.swap(_rocksdb_map);
    _channels.clear();
    _channels_complete = true;
  }

  for (auto& p : closing)
  {
    PUBSUB_LOG_BEGIN("Rocksdb close: " << p.first << "...")
    p.second->close();
    PUBSUB_LOG_END("Rocksdb close: " << p.first << " Done!")
  }
  _factory.reset();
}

multi_rocksdb::multi_rocksdb()
  : _factory(std::make_shared<rocksdb_factory>())
  , _channels_cache(false)
  , _channels_complete(true)
{

}

bool multi_rocksdb::configure( bool channels_cache, const rocksdb_options& opt)
{
  _factory->configure(opt);

  std::set<time_t> olds;
  {
    std::lock_guard<mutex_type> lk(_mutex);
    _channels_cache = channels_cache;
    if ( !channels_cache )
      _channels.clear();
    _channels_complete = true;
    std::transform(
      _rocksdb_map.begin(),
      _rocksdb_map.end(),
      std::inserter( olds, olds.begin() ),
      [](const rocksdb_map::value_type& v){
        return v.first;
      }
    );
  }

  for ( const auto& t : opt.ttl)
  {
    if ( auto db = _factory->create(t) )
    {
      olds.erase(t);
      std::lock_guard<mutex_type> lk(_mutex);
      _rocksdb_map[t] = db;
    }
    else
    {
      return false;
    }
  }

  for ( const auto& t : olds)
  {
    close_db_(t);
  }
  return true;
}

bool multi_rocksdb::push( const std::string& channel, const message& m )
{
  if ( m.lifetime == 0 && m.limit == 0 )
    return false;

  if ( auto db = this->get_db_(m.lifetime) )
  {
    if ( _channels_cache.load(std::memory_order_relaxed)
         && _channels_complete.load(std::memory_order_relaxed) )
    {
      std::lock_guard<mutex_type> lk(_mutex);
      if ( _channels_complete.load(std::memory_order_relaxed) )
      {
        if ( _channels.size() >= channels_cache_max )
        {
          _channels.clear();
          _channels_complete = false;
        }
        else
        {
          _channels.insert(channel);
        }
      }
    }
    return db->push(channel, m);
  }
  return false;
}

bool multi_rocksdb::get_messages( message_list_t* ml, const std::string& channel, cursor_t cursor, size_t limit)
{
  std::vector<rocksdb_ptr> dbs;
  {
    std::lock_guard<mutex_type> lk(_mutex);
    dbs.reserve(_rocksdb_map.size());
    for ( const auto& db : _rocksdb_map )
      dbs.push_back(db.second);
  }

  bool result = false;
  for ( const auto& db : dbs )
  {
    if ( db != nullptr )
      result |= db->get_messages(ml, channel, cursor, limit);
  }
  return result;
}

bool multi_rocksdb::has( const std::string& channel ) const
{
  if ( !_channels_cache.load(std::memory_order_relaxed) )
    return true;

  // После overflow кэш больше не гарантирует отсутствие канала
  if ( !_channels_complete.load(std::memory_order_relaxed) )
    return true;

  std::lock_guard<mutex_type> lk(_mutex);
  return 0!=_channels.count(channel);
}

multi_rocksdb::rocksdb_ptr multi_rocksdb::get_db_(time_t ttl) const
{
  std::lock_guard<mutex_type> lk(_mutex);

  if ( _rocksdb_map.empty() )
    return nullptr;

  auto itr = _rocksdb_map.lower_bound(ttl);
  if ( itr == _rocksdb_map.end() )
    return _rocksdb_map.rbegin()->second;

  return itr->second;
}

void multi_rocksdb::close_db_(time_t ttl)
{
  rocksdb_ptr closing;
  {
    std::lock_guard<mutex_type> lk(_mutex);
    auto itr = _rocksdb_map.find(ttl);
    if ( itr != _rocksdb_map.end() )
    {
      closing = itr->second;
      _rocksdb_map.erase(itr);
    }
  }
  if ( closing != nullptr )
    closing->close();
}

}}
