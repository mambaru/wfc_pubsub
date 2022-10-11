//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2013, 2014, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <pubsub/api/message.hpp>
#include <pubsub/api/actions.hpp>
#include <fas/system/memory.hpp>

namespace wfc{ namespace pubsub{


/** @brief Сообщение */
class stored_message
{
  message _m;
public:
  typedef std::unique_ptr<stored_message> ptr;

public:
  stored_message() = default;

  stored_message(const stored_message& other)
    : _m(stored_message::copy(other.get()))
  {
  };

  stored_message(stored_message&& other)
  {
    _m.action = other._m.action;
    _m.limit = other._m.limit;
    _m.birthtime = other._m.birthtime;
    _m.lifetime = other._m.lifetime;
    _m.cursor = other._m.cursor;
    _m.identity = std::move(other._m.identity);
    _m.persistent = other._m.persistent;
    _m.content = std::move(other._m.content);
    _m.key = std::move(other._m.key);
  };

  explicit stored_message(const message& m)
  {
    _m = this->copy(m);
  };

  explicit stored_message(message&& m)
  {
    _m.action = m.action;
    _m.limit = m.limit;
    _m.birthtime = m.birthtime;
    _m.lifetime = m.lifetime;
    _m.cursor = m.cursor;
    _m.identity = std::move(m.identity);
    _m.persistent = m.persistent;
    _m.content = std::move(m.content);
    _m.key = std::move(m.key);
  };

  static message copy(const message& m)
  {
    message _m;
    _m.action = m.action;
    _m.limit = m.limit;
    _m.birthtime = m.birthtime;
    _m.lifetime = m.lifetime;
    _m.cursor = m.cursor;
    _m.identity = m.identity;
    _m.persistent = m.persistent;
    _m.key = m.key;
    if ( m.content!=nullptr )
      _m.content = std::make_unique<content_t>(*m.content);
    return _m;
  }

  const message& get() const
  {
    return _m;
  }

  message get_copy() const
  {
    return stored_message::copy(_m);
  }

  message&& detach()
  {
    return std::move(_m);
  }

  actions action() const { return _m.action; }
  const cursor_t& cursor() const { return _m.cursor; }
  const identity_t& identity() const { return _m.identity; }
  const size_t& limit() const { return _m.limit;}
  bool unlimited() const { return _m.lifetime==0 || _m.limit == 0;}
  time_t lifetime() const { return _m.lifetime;}
  time_t birthtime() const { return _m.birthtime;}

  time_t deathtime() const
  {
    // если в lifetime время жизни (колько живет в сек)
    return _m.birthtime + _m.lifetime;
  }

  void reborn()
  {
    _m.birthtime = std::time(nullptr);
  }

  void reborn(time_t now)
  {
    _m.birthtime = now;
  }

  void update(const message& m)
  {
    _m.limit = m.limit;
    _m.birthtime = m.birthtime;
    _m.lifetime = m.lifetime;
    this->update_content( m );
    this->update_key( m );
  }

  void modify(const message& m)
  {
    _m.limit = m.limit;
    this->update_content( m );
    this->update_key( m );
  }

  void update_content(const message& m)
  {
    if ( m.content != nullptr )
    {
      if ( _m.content == nullptr )
      {
        _m.content = std::make_unique<content_t>( m.content->begin(), m.content->end() );
      }
      else
      {
        _m.content->assign( m.content->begin(), m.content->end() );
      }
    }
    else
    {
      _m.content.reset();
    }
  }

  void update_key(const message& m)
  {
    _m.key = m.key;
  }
};

}}

