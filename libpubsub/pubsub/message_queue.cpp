//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//

#include <deque>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "api/types.hpp"
#include "api/message.hpp"
#include "message_queue.hpp"

#include <deque>
#include <algorithm>
#include <iostream>


namespace wfc{ namespace pubsub{

struct f_deathtime_less
{
  time_t now;

  f_deathtime_less(time_t now1): now(now1) {}

  bool operator()( const stored_message::ptr& m) const
  {
    return m->deathtime() < now;
  }
};

struct f_identity_equal
{
  const message& m;

  f_identity_equal(const message& m1)
    : m(m1)
  {
  }

  bool operator()( const stored_message::ptr& msg) const
  {
    return msg->identity() == m.identity;
  }
};

struct f_modify_if_identity_equal
{
  const message& m;
  // возможное нарушение последовательности
  bool possible_violation_of_sequence;
  int count;
  f_modify_if_identity_equal(const message& m1)
    : m(m1)
    , possible_violation_of_sequence(false)
    , count(0)
  {
  }

  void operator() (stored_message::ptr& msg)
  {
    if (msg->identity() == m.identity )
    {
      if ( !possible_violation_of_sequence && msg->limit() != m.limit )
        possible_violation_of_sequence = true;

      msg->modify(m);
      ++count;
    }
  }
};

struct f_update_if_identity_equal
{
  time_t now;
  const message& m;
  int count;

  f_update_if_identity_equal( time_t now1, const message& m1 )
    : now(now1)
    , m(m1)
    , count(0)
  {
    
  }

  void operator() (stored_message::ptr& msg)
  {
    if (msg->identity() == m.identity )
    {
      msg->update(m);
      ++count;
    }
  }

};


message_queue::message_queue()
  : _sequenced_flag(true)
  , _last_remove_death(0)
  , _removed(0)
{}

size_t message_queue::size() const
{
  return _queue.size();
}

size_t message_queue::get_removed(bool reset) const
{
  size_t res = _removed;
  if (reset)
    _removed = 0 ;
  return res;
}


bool message_queue::empty() const
{
  return _queue.empty();
}

void message_queue::clear()
{
  _queue.clear();
}

bool message_queue::push( message&& m)
{
  return this->push( std::make_unique<stored_message>( std::move(m) ))==nullptr;
}

stored_message::ptr message_queue::push( stored_message::ptr m)
{
  time_t now = time(nullptr);
  return this->push( now, std::move(m) );
}

bool message_queue::push( time_t now,  message&& m)
{
  return this->push( now, std::make_unique<stored_message>( std::move(m) ) )==nullptr;
}


stored_message::ptr message_queue::push( time_t now, stored_message::ptr m)
{
  /** Сначала удаляем устаревшие сообщения */
  if ( _last_remove_death != now)
  {
    this->remove_death(now);
    _last_remove_death = now;
  }

  if ( m->action() == actions::publish )
  {
    /** Размер очереди установлен в 0 - сообщение в очередь не ставиться, а сразу доставляется клиенту, очередь очищается */
    if ( m->limit() == 0)
      this->resize(0);

    /** Время жизни 0 - сообщение в очередь не ставиться, а сразу доставляется клиенту */
    if ( m->unlimited() )
      return m;
  }

  size_t limit = m->limit();
  
  switch( m->action() )
  {
    case actions::publish: publish_(now, std::move(m) ); break;
    case actions::modify:  modify_(now, std::move(m) ); break;
    case actions::update:  update_(now, std::move(m) ); break;
    case actions::replace: replace_(now, std::move(m) ); break;
    case actions::remove:  remove_(now, std::move(m) ); break;
    default: throw std::logic_error("message_queue::push: unknown method");
  }

  /** удаляем наиболее старые сообщения, даже если время жизни их не истекло */
  this->resize(limit);
  return nullptr;
}


void message_queue::resize(size_t limit)
{
  if ( _queue.size() <= limit)
    return;
  queue_type::iterator itr = _queue.begin();
  std::advance( itr, _queue.size() - limit);
  _removed += _queue.size();
  _queue.erase(_queue.begin(), itr);
  _removed -= _queue.size();
}

void message_queue::remove_death()
{
  remove_death(time(nullptr));
}

/// @brief Удаляет сообщения время жизни которых закончилось относительно заданного времени
  /**
   * @param now время относительно которого будет происходить проверка
   * @return количество удаленных сообщений
  */
void message_queue::remove_death(time_t now)
{
  f_deathtime_less fdp(now);

  bool has_removed = false;


  // Сначала удаляем устаревшие из начала очереди
  while ( !_queue.empty() )
  {
    if ( fdp(_queue.front()) )
    {
      ++_removed;
      _queue.pop_front();
    }
    else
    {
      break;
    }
    has_removed = true;
  }

  // Если последовательность времен жизни нарушена
  if ( !_sequenced_flag && !_queue.empty())
  {
    size_t size = _queue.size();
    // Пробегаем по всей очереди и удаляем старые
    _queue.erase( std::remove_if(_queue.begin(), _queue.end(), fdp), _queue.end());
    has_removed |=  (size != _queue.size());
    _removed += size - _queue.size();
    if ( has_removed )
    {
      // Определяем восстановилась ли упорядоченность
      if ( _queue.size() > 1)
      {
        queue_type::iterator beg = _queue.begin();
        queue_type::iterator end = _queue.end();
        --end;
        for ( _sequenced_flag = true;  _sequenced_flag && beg!=end; ++beg )
        {
          queue_type::iterator itr = beg + 1;
          _sequenced_flag = ( (*beg)->deathtime() == (*itr)->deathtime() ) && ( (*beg)->birthtime() <= (*itr)->birthtime() );
        }
      }
      else
        _sequenced_flag = true;
    }
  }
}

bool message_queue::sequenced() const
{
  return _sequenced_flag;
}

message_queue::queue_type::iterator message_queue::insert_( time_t now, stored_message::ptr m )
{
  // Находим позицию
  queue_type::iterator itr = std::upper_bound(_queue.begin(), _queue.end(), m, f_cursor_less() );

  // Вставляем
  itr = _queue.insert(itr, std::move(m) );
  if (itr==_queue.end())
    return itr;

  if ( (*itr)->birthtime() == 0 )
    (*itr)->reborn(now);

  return itr;
}

void message_queue::publish_(time_t now, stored_message::ptr m)
{
  queue_type::iterator itr = insert_(now, std::move(m) );
  if ( itr==_queue.end() )
    return;

  queue_type::iterator mitr = itr;

// Проверяем упороядоченность сообщений по времени жизни
  // (для ускорения удаления)
  if ( _sequenced_flag )
  {
    if ( itr!=_queue.begin() )
    {
      --itr;
      _sequenced_flag = ( (*itr)->deathtime() == (*mitr)->deathtime() ) && ( (*itr)->birthtime() <= now );
      ++itr;
    }
    ++itr;
    if ( _sequenced_flag && itr!=_queue.end())
    {
      _sequenced_flag = ( (*itr)->deathtime() == (*mitr)->deathtime() ) && ( (*itr)->birthtime() >= now );
    }
  }
}

/// @brief обновляет все сообшения с обновлением времени создания
void message_queue::update_(time_t now, stored_message::ptr m)
{
  f_update_if_identity_equal f = std::for_each(_queue.begin(), _queue.end(), f_update_if_identity_equal(now, m->get() ) );
  if ( f.count > 0 )
    _sequenced_flag = false;
  if ( m->lifetime()!=0 && f.count == 0 )
    publish_( now, std::move(m) );
}

/// @brief обновляет все сообшения без обновления времени создания
void message_queue::modify_(time_t now, stored_message::ptr m )
{
  f_modify_if_identity_equal f = std::for_each(_queue.begin(), _queue.end(), f_modify_if_identity_equal( m->get() ) );

  if ( _sequenced_flag && f.possible_violation_of_sequence )
    _sequenced_flag = false;

  if ( m->lifetime()!=0 && f.count == 0)
    publish_( now, std::move(m) );
}

/// @brief Удаляет все сообщения по identity
void message_queue::remove_( time_t now, stored_message::ptr m)
{
  this->remove_(now, m->get() );
  
  if ( m->lifetime()!=0 )
    publish_( now, std::move(m) );
}

void message_queue::remove_( time_t /*now*/, const message& m)
{
  _queue.erase(
    std::remove_if(
      _queue.begin(),
      _queue.end(),
      f_identity_equal(m)
    ),
    _queue.end()
  );
}

/// @brief
void message_queue::replace_(time_t now, stored_message::ptr m )
{
  remove_(now, m->get());
  publish_(now, std::move(m) );
}

}}


