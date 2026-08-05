#include "pubsub.hpp"
#include "logger.hpp"
#include "rocksdb/multi_rocksdb.hpp"
#include <message_queue/stored_message.hpp>
#include <fas/system/memory.hpp>

namespace wfc{ namespace pubsub{

std::atomic<subscriber_id_t> pubsub::_id_counter(0);

pubsub::pubsub()
{
}

void pubsub::reconfigure(rocksdb_ptr db, const pubsub_options& opt)
{
  _opt = opt;
  _rocksdb = db;
}

pubsub::fire_fun_t pubsub::publish(const std::string& channel, message&& msg)
{
  if ( !(msg.lifetime==0 && msg.limit==0) )
  {
    if ( msg.birthtime == 0 )
      msg.birthtime = std::time(nullptr);

    if ( msg.lifetime==0  )
      msg.lifetime = _opt.default_lifetime;

    if ( msg.limit==0  )
      msg.limit = _opt.default_limit;

    if ( msg.persistent || _opt.persistent_default)
    {
      if (!_opt.rocksdb_disabled && _rocksdb!=nullptr)
      {
        _rocksdb->push(channel, msg);
      }
    }

    if ( !_opt.cache_disabled && !msg.persistent )
      _channel_map.push( channel, msg);
  }

  return this->fire_(channel, msg );
}

void pubsub::get_messages(message_list_t* ml, const subscribe_params& params)
{
  if (!_opt.rocksdb_disabled && _rocksdb!=nullptr)
  {
    _rocksdb->get_messages(ml, params.channel, params.cursor, params.limit);
    if ( !ml->empty() && _opt.persistent_default )
      return;
  }

  if (!_opt.cache_disabled)
    _channel_map.get_messages(ml, params.channel, params.cursor, params.limit);
}


subscriber_id_t pubsub::subscribe(message_list_t* ml, const subscribe_params& params)
{
  ++_id_counter;
  this->subscribe( ml, _id_counter, params);
  return _id_counter;
}

void pubsub::subscribe(message_list_t* ml, subscriber_id_t id, const subscribe_params& params)
{
  this->describe_(id, params.channel);
  this->subscribe_(id, params.channel, params.handler);
  this->get_messages(ml, params);
}

size_t pubsub::describe(subscriber_id_t id)
{
  auto beg = _subscriber_set.lower_bound({id, ""});
  auto itr = beg;
  for (;itr!=_subscriber_set.end() && itr->first==id; ++itr)
  {
    _handler_map.erase({itr->second, itr->first});
  }
  std::ptrdiff_t count = std::distance(beg, itr);
  _subscriber_set.erase(beg, itr);

  return static_cast<size_t>(count);
}

bool pubsub::describe(subscriber_id_t id, const std::string& channel)
{
  return this->describe_(id, channel);
}

bool pubsub::describe_(subscriber_id_t id, const std::string& channel)
{
  auto itr = _subscriber_set.lower_bound({id, channel});
  if ( itr == _subscriber_set.end() )
    return false;

  if ( itr->first != id || itr->second!=channel )
    return false;

  _handler_map.erase({itr->second, itr->first});
  _subscriber_set.erase(itr);
  return true;
}

size_t pubsub::subscriber_count() const
{
  if (_handler_map.size() != _subscriber_set.size() )
    abort();
  return _handler_map.size();
}


size_t pubsub::size(size_t* count) const
{
  return _channel_map.size(count);
}

size_t pubsub::removed_count(bool reset) const
{
  return _channel_map.removed_count(reset);
}

size_t pubsub::empty_count() const
{
  return _channel_map.empty_count();
}

bool pubsub::subscribe_(subscriber_id_t id, const std::string& channel, const handler_fun_t& handler)
{
  _subscriber_set.insert({id, channel});
  return _handler_map.insert({{channel, id}, std::make_shared<handler_fun_t>(handler)}).second;
}

pubsub::fire_fun_t pubsub::fire_(const std::string& channel, const message& msg) const
{
  auto pch  = std::make_shared<std::string>(channel);
  auto pmsg = std::make_shared<message>( message::copy(msg) );
  auto fun_list = std::make_shared<std::deque<fire_fun_t>>();
  auto itr = _handler_map.lower_bound({channel, 0});
  for(;itr != _handler_map.end() && itr->first.first==channel; ++itr)
  {
    std::weak_ptr<handler_fun_t> wfun = itr->second;
    fun_list->push_back(
      [wfun, pch, pmsg]()
      {
        // Чтобы уменьшить вероятность рассылки отписанным
        if ( auto pfun = wfun.lock())
          (*pfun)( *pch, *pmsg );
      }
    );
  }
  return [fun_list]()
  {
    for (const auto& f : *fun_list)
      f();
  };
}

void pubsub::reset()
{
  _subscriber_set.clear();
  _handler_map.clear();
  _channel_map.clear(nullptr);
}

void pubsub::write_log() const
{
}

}}

