#include "pubsub_mt.hpp"
#include "logger.hpp"
#include "rocksdb/multi_rocksdb.hpp"
#include <algorithm>

namespace wfc{ namespace pubsub{

pubsub_mt::pubsub_mt(const pubsub_options& opt, const rocksdb_options& ropt)
  : _size(opt.hash_size)
{
  if ( _size == 0 )
    _size = 1;

  _mutext_list = std::vector<mutex_type>(_size);
  _pubsub_list = std::vector<pubsub>(_size);

  if ( !ropt.path.empty() )
  {
    _rocksdb = std::make_shared<multi_rocksdb>();
    _rocksdb->configure( opt.persistent_default == false, ropt);
  }
  for (auto& ps : _pubsub_list)
    ps.reconfigure(_rocksdb, opt);
}


void pubsub_mt::close()
{
  if ( _rocksdb!=nullptr )
    _rocksdb->close();
}

void pubsub_mt::reconfigure(const pubsub_options& , const rocksdb_options& )
{
  PUBSUB_LOG_WARNING("pubsub_mt::reconfigure NOT IMPLEMENTED!!!")
}

void pubsub_mt::get_messages(message_list_t* ml, const subscribe_params& params)
{
  size_t index = index_(params.channel);
  std::lock_guard<mutex_type> lk(_mutext_list[index]);
  return _pubsub_list[index].get_messages(ml, params);
}

void pubsub_mt::publish(const std::string& channel, message&& msg)
{
  size_t index = index_(channel);
  std::lock_guard<mutex_type> lk(_mutext_list[index]);
  _pubsub_list[index].publish(channel, std::move(msg));
}

subscriber_id_t pubsub_mt::subscribe(message_list_t* ml, const subscribe_params& params)
{
  size_t index = index_(params.channel);
  std::lock_guard<mutex_type> lk(_mutext_list[index]);
  return _pubsub_list[index].subscribe(ml, params);
}

void pubsub_mt::subscribe(message_list_t* ml, subscriber_id_t id, const subscribe_params& params)
{
  size_t index = index_(params.channel);
  std::lock_guard<mutex_type> lk(_mutext_list[index]);
  _pubsub_list[index].subscribe(ml, id, params);
}

size_t pubsub_mt::describe(subscriber_id_t id)
{
  size_t count = 0;
  for (size_t i = 0 ; i < _size; ++i)
  {
    std::lock_guard<mutex_type> lk(_mutext_list[i]);
    count+= _pubsub_list[i].describe(id);
  }
  return count;
}

bool pubsub_mt::describe(subscriber_id_t id, const std::string& channel)
{
  size_t index = index_(channel);
  std::lock_guard<mutex_type> lk(_mutext_list[index]);
  return _pubsub_list[index].describe(id, channel);
}

size_t pubsub_mt::subscriber_count() const
{
  size_t count = 0;
  for (size_t i = 0 ; i < _size; ++i)
  {
    std::lock_guard<mutex_type> lk(_mutext_list[i]);
    count+= _pubsub_list[i].subscriber_count();
  }
  return count;
}

void pubsub_mt::write_log() const
{
  for (size_t i = 0 ; i < _size; ++i)
  {
    std::lock_guard<mutex_type> lk(_mutext_list[i]);
    _pubsub_list[i].write_log();
  }
}

void pubsub_mt::reset()
{
  for (size_t i = 0 ; i < _size; ++i)
  {
    std::lock_guard<mutex_type> lk(_mutext_list[i]);
    _pubsub_list[i].reset();
  }
}


size_t pubsub_mt::size(size_t* count) const
{
  size_t s = 0;
  for (size_t i = 0 ; i < _size; ++i)
  {
    std::lock_guard<mutex_type> lk(_mutext_list[i]);
    s += _pubsub_list[i].size(count);
  }
  return s;
}

size_t pubsub_mt::get_removed(bool reset) const
{
  size_t s = 0;
  for (size_t i = 0 ; i < _size; ++i)
  {
    std::lock_guard<mutex_type> lk(_mutext_list[i]);
    s += _pubsub_list[i].get_removed(reset);
  }
  return s;
}

size_t pubsub_mt::empty_count() const
{
  size_t s = 0;
  for (size_t i = 0 ; i < _size; ++i)
  {
    std::lock_guard<mutex_type> lk(_mutext_list[i]);
    s += _pubsub_list[i].empty_count();
  }
  return s;
}

size_t pubsub_mt::index_( const std::string& channel ) const
{
  return std::hash<std::string>()(channel) % _size ;
}


}}

