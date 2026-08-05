#include "channel_map.hpp"
#include<list>

namespace wfc{ namespace pubsub{

channel_map::~channel_map()
{

}

channel_map::channel_map()
{

}

size_t channel_map::size(size_t* count) const
{
  if ( count != nullptr )
  {
    for ( const auto& ch : _channel_map)
    {
      *count += ch.second.size();
    }
  }
  return _channel_map.size();
}

void channel_map::clear(std::vector<std::string>* channels)
{
  for ( auto& ch : _channel_map)
  {
    if ( channels!=nullptr )
      channels->push_back(ch.first);
    ch.second.clear();
  }
  _channel_map.clear();
}

size_t channel_map::removed_count(bool reset) const
{
  size_t s = 0;
  for ( const auto& ch : _channel_map)
  {
    s += ch.second.removed_count(reset);
  }
  return s;
}

size_t channel_map::empty_count() const
{
  size_t s = 0;
  for ( const auto& ch : _channel_map)
  {
    s += static_cast<size_t>( ch.second.empty() );
  }
  return s;
}


size_t channel_map::channel_size(const std::string& name) const
{
  auto itr = _channel_map.find( name );
  if ( itr != _channel_map.end() )
    return itr->second.size();
  return 0;
}

stored_message::ptr channel_map::push( const std::string& name, const message& m )
{
  auto itr = _channel_map.find( name );
  if ( itr == _channel_map.end() )
  {
    if ( m.lifetime == 0 && m.limit==0 )
      return std::make_unique<stored_message>(m);

    itr = _channel_map.insert( std::make_pair(name, message_queue() ) ).first;
  }
  return itr->second.push( message::copy(m) );
}


void channel_map::get_messages( message_list_t* ml, const std::string& name )
{
  auto itr = _find_and_remove_death(name);
  if (itr == _channel_map.end())
    return;

  if ( ml!=nullptr )
    itr->second.copy_to( std::back_inserter(*ml) );

}

void channel_map::get_messages( message_list_t* ml, const std::string& name, cursor_t cursor )
{
  auto itr = _find_and_remove_death(name);
  if (itr == _channel_map.end())
    return ;

  if ( ml!=nullptr )
    itr->second.copy_to( cursor, std::back_inserter(*ml) );
}

void channel_map::get_messages( message_list_t* ml, const std::string& name, cursor_t cursor, size_t limit)
{
  auto itr = _find_and_remove_death(name);
  if (itr == _channel_map.end())
    return;

  if ( ml!=nullptr )
  {
    itr->second.copy_to( cursor, limit, std::back_inserter(*ml) );
  }
}

void channel_map::erase(const std::string& name)
{
  _channel_map.erase( name );
}

channel_map::channel_map_t::iterator channel_map::_find_and_remove_death( const std::string& name )
{
  channel_map_t::iterator itr = _channel_map.find( name );
  if ( itr == _channel_map.end() )
    return itr;
  itr->second.remove_death();
  if ( itr->second.empty() )
  {
    _channel_map.erase(itr);
    return _channel_map.end();
  }
  return itr;
}

size_t channel_map::takeaway(size_t limit, topic_list_t* tl)
{
  time_t now = time(nullptr);

  size_t count = 0;
  for (auto& [channel, mq] : _channel_map )
  {
    message_list_t ml;
    mq.remove_death(now);
    ml.reserve( limit < mq.size() ? limit : ml.size());
    limit -= mq.takeaway(limit, std::back_inserter(ml));

    std::string ch = channel;
    std::transform(ml.begin(), ml.end(), std::back_inserter(*tl), [ch, &count](message& m) -> topic  {
      ++count;
      topic tp;
      tp.channel = ch;
      static_cast<message&>(tp) = std::move(m);
      return tp;
    });
  }
  return count;
}

size_t channel_map::remove_death()
{
  size_t count = 0;
  std::list<std::string> empty_list;
  for (auto& [name, mq] : _channel_map )
  {
    count += mq.remove_death();
    if ( mq.empty() )
      empty_list.push_back(name);
  }

  for ( const auto& name : empty_list)
  {
    _channel_map.erase(name);
  }
  return count;
}

}}
