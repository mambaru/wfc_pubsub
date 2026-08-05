#include "wait_list.hpp"
#include "logger.hpp"

namespace wfc{ namespace pubsub{

wait_list::wait_list(time_t timeout_s)
  : _timeout_s(timeout_s)
{
}

void wait_list::wait(const std::vector<std::string>& cl)
{
  for (const std::string& channel: cl)
  {
    this->ready(channel);
    time_t ttl = time(nullptr) + _timeout_s;
    _channel_map[channel] = ttl;
    _time_set.insert( std::make_pair(ttl, channel) );
  }
}

void wait_list::ready(const std::string& name)
{
  auto itr = _channel_map.find(name);
  if ( itr == _channel_map.end() )
    return;

  _time_set.erase( std::make_pair(itr->second, itr->first) );
  _channel_map.erase(itr);
}

void wait_list::ready(const std::vector<std::string>& cl)
{
  for (const std::string& channel: cl)
    this->ready(channel);
}

size_t wait_list::pop_outdated(std::set<std::string>* cl)
{
  size_t count = 0;
  time_t now = time(nullptr);
  for (;; ++count)
  {
    if ( _time_set.empty() )
      return count;

    if ( now < _time_set.begin()->first  )
      return count;

    auto channel = _time_set.begin()->second;
    cl->insert( channel );
    this->ready( channel );
  }
  return count;
}

void wait_list::clear()
{
  _channel_map.clear();
  _time_set.clear();
}

size_t wait_list::size() const
{
  if ( _channel_map.size() != _time_set.size() )
  {
    LONGPOLL_LOG_ERROR("Нарушена консистентность wait_list subscribe_manager " << _channel_map.size() << "!=" << _time_set.size() );
  }

  return _time_set.size();
}

}}
