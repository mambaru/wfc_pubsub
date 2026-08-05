#pragma once

#include <string>
#include <vector>

namespace wfc{ namespace pubsub{

struct publish_options
{
  // Разрешить клиентам publish через longpolling-service
  bool enabled = false;

  // Префиксы каналов при enabled=true; ["*"] — любые каналы
  std::vector<std::string> channel_prefixes;

  bool allows_channel(const std::string& channel) const
  {
    if ( !enabled || channel_prefixes.empty() )
      return false;

    for ( const auto& prefix : channel_prefixes )
    {
      if ( prefix == "*" )
        return true;
      if ( channel.size() >= prefix.size()
        && channel.compare(0, prefix.size(), prefix) == 0 )
        return true;
    }
    return false;
  }

  /// Все сообщения (элементы с полем .channel) разрешены префиксами
  template<typename Messages>
  bool allows_all(const Messages& messages) const
  {
    if ( !enabled || channel_prefixes.empty() )
      return false;

    for ( const auto& m : messages )
    {
      if ( !this->allows_channel(m.channel) )
        return false;
    }
    return true;
  }
};

}}
