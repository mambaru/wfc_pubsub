//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "hub_rpc_inflight.hpp"

namespace wfc{ namespace pubsub{

void hub_rpc_inflight::configure(size_t subscribe_max, size_t describe_max, time_t timeout_s)
{
  _subscribe_max = subscribe_max == 0 ? std::numeric_limits<size_t>::max() : subscribe_max;
  _describe_max = describe_max == 0 ? std::numeric_limits<size_t>::max() : describe_max;
  _timeout_s = timeout_s > 0 ? timeout_s : 10;
  this->reset();
}

void hub_rpc_inflight::reset()
{
  _subscribe = 0;
  _describe = 0;
  _subscribe_deadline.clear();
  _describe_deadline.clear();
}

void hub_rpc_inflight::recover()
{
  const time_t now = time(nullptr);
  while ( !_subscribe_deadline.empty() && _subscribe_deadline.front() <= now )
  {
    _subscribe_deadline.pop_front();
    if ( _subscribe.load() > 0 )
      _subscribe.fetch_sub(1);
  }
  while ( !_describe_deadline.empty() && _describe_deadline.front() <= now )
  {
    _describe_deadline.pop_front();
    if ( _describe.load() > 0 )
      _describe.fetch_sub(1);
  }
}

bool hub_rpc_inflight::subscribe_full() const
{
  return _subscribe.load() >= _subscribe_max;
}

bool hub_rpc_inflight::describe_full() const
{
  return _describe.load() >= _describe_max;
}

size_t hub_rpc_inflight::subscribe_count() const
{
  return _subscribe.load();
}

size_t hub_rpc_inflight::describe_count() const
{
  return _describe.load();
}

size_t hub_rpc_inflight::subscribe_max() const
{
  return _subscribe_max;
}

size_t hub_rpc_inflight::describe_max() const
{
  return _describe_max;
}

void hub_rpc_inflight::note_subscribe()
{
  _subscribe_deadline.push_back( time(nullptr) + _timeout_s );
  ++_subscribe;
}

void hub_rpc_inflight::note_describe()
{
  _describe_deadline.push_back( time(nullptr) + _timeout_s );
  ++_describe;
}

void hub_rpc_inflight::ack_subscribe()
{
  if ( !_subscribe_deadline.empty() )
  {
    _subscribe_deadline.pop_front();
    if ( _subscribe.load() > 0 )
      _subscribe.fetch_sub(1);
  }
}

void hub_rpc_inflight::ack_describe()
{
  if ( !_describe_deadline.empty() )
  {
    _describe_deadline.pop_front();
    if ( _describe.load() > 0 )
      _describe.fetch_sub(1);
  }
}

}}
