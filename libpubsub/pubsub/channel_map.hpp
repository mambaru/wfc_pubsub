//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2013, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <pubsub/api/message.hpp>
#include <pubsub/message_queue.hpp>

#include <set>
#include <map>
#include <stdexcept>
#include <iostream>


namespace wfc{ namespace pubsub{

/** @brief Канал */
class channel_map
{
  typedef std::map<std::string, std::unique_ptr<message_queue> > channel_map_t;

public:
  typedef std::vector<message> message_list_t;

  virtual ~channel_map();
  channel_map();
  channel_map(const channel_map& ) = delete;
  channel_map& operator=(const channel_map& ) = delete;

  size_t channel_size(const std::string& name) const;

  size_t size(size_t* count) const;

  size_t get_removed(bool reset) const;

  size_t empty_count() const;

  bool push( const std::string& name, const message& m );

  void get_messages( message_list_t* ml, const std::string& name);

  void get_messages( message_list_t* ml, const std::string& name, cursor_t cursor);

  void get_messages( message_list_t* ml, const std::string& name, cursor_t cursor, size_t limit);

  void erase(const std::string& name);

  void clear();
private:

  channel_map_t::iterator _find_and_remove_death( const std::string& name );
  channel_map_t   _channel_map;
};

}}



