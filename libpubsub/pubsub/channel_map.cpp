#include "channel_map.hpp"

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
      *count += ch.second->size();
    }
  }
  return _channel_map.size();
}

void channel_map::clear()
{
  _channel_map.clear();
}

size_t channel_map::get_removed(bool reset) const
{
  size_t s = 0;
  for ( const auto& ch : _channel_map)
  {
    s += ch.second->get_removed(reset);
  }
  return s;
}

size_t channel_map::empty_count() const
{
  size_t s = 0;
  for ( const auto& ch : _channel_map)
  {
    s += static_cast<size_t>( ch.second->empty() );
  }
  return s;
}


size_t channel_map::channel_size(const std::string& name) const
{
  auto itr = _channel_map.find( name );
  if ( itr != _channel_map.end() )
    return itr->second->size();
  return 0;
}

bool channel_map::push( const std::string& name, const message& m )
{
  auto itr = _channel_map.find( name );
  if ( itr == _channel_map.end() )
  {
    if ( m.lifetime == 0 && m.limit==0 )
      return false;

    itr = _channel_map.insert( std::make_pair(name, std::make_unique<message_queue>() ) ).first;
  }
  return itr->second->push( stored_message::copy(m) );
}


void channel_map::get_messages( message_list_t* ml, const std::string& name )
{
  auto itr = _find_and_remove_death(name);
  if (itr == _channel_map.end())
    return;

  if ( ml!=nullptr )
    itr->second->copy_to( std::back_inserter(*ml) );

}

void channel_map::get_messages( message_list_t* ml, const std::string& name, cursor_t cursor )
{
  auto itr = _find_and_remove_death(name);
  if (itr == _channel_map.end())
    return ;

  if ( ml!=nullptr )
    itr->second->copy_to( cursor, std::back_inserter(*ml) );
}

void channel_map::get_messages( message_list_t* ml, const std::string& name, cursor_t cursor, size_t limit)
{
  auto itr = _find_and_remove_death(name);
  if (itr == _channel_map.end())
    return;

  if ( ml!=nullptr )
  {
    itr->second->copy_to( cursor, limit, std::back_inserter(*ml) );
  }
}

void channel_map::erase(const std::string& name)
{
  _channel_map.erase( name );
}

channel_map::channel_map_t::iterator channel_map::_find_and_remove_death( const std::string& name )
{
  time_t now = time(nullptr);
  channel_map_t::iterator itr = _channel_map.find( name );
  if ( itr == _channel_map.end() )
    return itr;
  itr->second->remove_death(now);
  if ( itr->second->empty() )
  {
    _channel_map.erase(itr);
    return _channel_map.end();
  }
  return itr;
}

}}
