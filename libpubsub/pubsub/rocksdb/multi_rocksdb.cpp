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
  for (auto& p : _rocksdb_map)
  {
    PUBSUB_LOG_BEGIN("Rocksdb close: " << p.first << "...")
    p.second->close();
    PUBSUB_LOG_END("Rocksdb close: " << p.first << " Done!")
  }
  _rocksdb_map.clear();
  _factory.reset();
}

multi_rocksdb::multi_rocksdb()
  : _factory(std::make_shared<rocksdb_factory>())
  , _channels_cache(false)
{

}

bool multi_rocksdb::configure( bool channels_cache, const rocksdb_options& opt)
{
  _factory->configure(opt);

  std::set<time_t> olds;
  {
    std::lock_guard<mutex_type> lk(_mutex);
    _channels_cache = channels_cache;
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
    if (_channels_cache)
    {
      std::lock_guard<mutex_type> lk(_mutex);
      _channels.insert(channel);
    }
    return db->push(channel, m);
  }
  return false;
}

bool multi_rocksdb::get_messages( message_list_t* ml, const std::string& channel, cursor_t cursor, size_t limit)
{
  bool result = false;
  for ( const auto& db : _rocksdb_map )
  {
    result |= db.second->get_messages(ml, channel, cursor, limit);
  }
  return result;
}

bool multi_rocksdb::has( const std::string& channel ) const
{
  if ( !_channels_cache )
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
  std::lock_guard<mutex_type> lk(_mutex);
  auto itr = _rocksdb_map.find(ttl);
  if ( itr != _rocksdb_map.end() )
  {
    itr->second->close();
    _rocksdb_map.erase(itr);
  }
}

}}
