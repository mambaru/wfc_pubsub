//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once


#include <pubsub/stored_message.hpp>
#include <deque>
#include <algorithm>
#include <memory>

namespace wfc{ namespace pubsub{


/**
 * @brief Очередь сообщений message 
 *
 * Хранимые сообщения отсортированы по полю cursor.
 * Сообщения с одинаковым cursor распологаются в очереди в порядке поступления.
 * Перед выполнением большинства методов удаляются устаревшие сообщения
 *
 * Важно! Размер очереди сообщений управляется входящими сообщениям (атрибут limit). Если
 * размер очереди болше limit входящего сообщения, то удаляются наиболее старые
 *
 * Для опреции publish cообщения с нулевым временем жизни ( lifetime == 0 ) в очередь не помещаются
 * Сообщения с limit равным 0 также не помещаются в очередь, при этом сама очередь очищается
 * 
 * При помещение сообщения в очередь, его поле birthtime заменяется текущим временем.
 * Это поле не должно использоваться клиентом, т.к. это время не создания сообщения,
 * а помещения его в эту очередь. За время пути сообщение может пройти через несколько 
 * очередей или не попасть ни в одну.
 *
 * Идеальный вариант размещения сообщений в очереди:
 *   a) все сообщения очереди имеют одинаковый или возрастающий cursor
 *   б) все сообщения очереди имеют одинаковое время жизни lifetime
 *   в) все сообщения очереди имеют возрастающий birthtime *
 *                * (последовательность может быть нарушена командой update)
 *
 * В этом случай очередь считается упорядоченной по времени, и операция удаления
 * устаревших сообщений происходит быстрее.
 *
 * Очередь вновь будет считается упорядоченной при удалнии сообщений нарушающих эти условия
 * (проверка присходит при удалении устаревших сообщений)
 */
class message_queue
{
public:
  typedef std::deque<stored_message::ptr> queue_type;

  struct f_cursor_less
{
  bool operator()(const stored_message::ptr& left, const stored_message::ptr& right) const
  {
    return left->cursor() < right->cursor();
  }

  bool operator()(cursor_t left, const stored_message::ptr& right) const
  {
    return left < right->cursor();
  }

  bool operator()(const stored_message::ptr& left, cursor_t right) const
  {
    return left->cursor() < right;
  }
};

public:
  message_queue();

  size_t size() const;
  
  size_t get_removed(bool reset) const;

  bool empty() const;
  
  void clear();

  /** @brief Добавить сообщение, удалить или обновить сообщения
   *  @param m входящее сообщение
   *
   *  В зависимости от поля message::action сообщения, добавляет, удаляет или обновляет сообщения в канале:
   *    publish - добавляет сообщение в очередь
   *    modify  - обновляет контент сообщений и время жизни.
   *    update  - обновляет контент сообщений и время жизни, поле birthtime устанавливается в текущее время
   *    replace - заменяет сообщения (комбинация remove и publish)
   *    remove  - удаляет сообщения
   *
   * Опреации modify, update, replace и remove исползуют поле identity для идентификации сообщений.
   * При выполнении каждой из перечисленных операций происходит проход по всей очереди сообщений и 
   * сверяется аттрибут identity.
   * 
   * Для оперции remove достаточно правильно указать поле identity
   *
   * TODO:??? доставлять ли их клиенту, если доставлять то как?
   */
  bool push( message&& m);

  stored_message::ptr push( stored_message::ptr m);
  

  /// @brief Добавляет сообщение в канал
  /** @return текущее количество сообщений в канале
   *
   * После добавления вызывает функцию remove_death
   * return nullptr - если остался в канале
   */
  bool push( time_t now, message&& m);
  stored_message::ptr push( time_t now, stored_message::ptr m);

  void resize(size_t limit);

  /// @brief Удаляет сообщения время жизни которых закончилось
  /** @return количество удаленных сообщений*/
  void remove_death();

  /// @brief Удаляет сообщения время жизни которых закончилось относительно заданного времени
  /**
   * @param now время относительно которого будет происходить проверка 
   * @return количество удаленных сообщений
  */
  void remove_death(time_t now);

  bool sequenced() const ;

  // не забываем делать remove_death() перед копированием 
  template<typename Itr>
  void copy_to(Itr itr)
  {
    std::transform(_queue.begin(), _queue.end(), itr, [](const stored_message::ptr& ptr) -> message {
      return ptr->get_copy();
    });
  }

  // не забываем делать remove_death() перед копированием 
  template<typename Itr>
  void copy_to(cursor_t cursor, Itr itr) const
  {
    queue_type::const_iterator beg = std::lower_bound(_queue.begin(), _queue.end(), cursor, f_cursor_less() );
    std::transform(beg, _queue.end(), itr, [](const stored_message::ptr& ptr) -> message {
      return ptr->get_copy();
    });
  }

  // не забываем делать remove_death() перед копированием 
  template<typename Itr>
  void copy_to(cursor_t cursor, size_t limit, Itr itr) const
  {
    queue_type::const_iterator beg = std::lower_bound(_queue.begin(), _queue.end(), cursor, f_cursor_less() );
    queue_type::const_iterator end = _queue.end();
    size_t s = static_cast<size_t>(std::distance(beg, end));
    if ( limit < s )
      std::advance( beg, s - limit);
    std::transform(beg, end, itr, [](const stored_message::ptr& ptr) -> message  {
      return ptr->get_copy();
    });
  }

private:

  queue_type::iterator insert_( time_t now, stored_message::ptr m );
  
  /**
   * @brief добавляет сообщение в канал, в позицию cursor.
   * @param now текущее время
   * @param m публикуемое сообщение
   *
   * В канале сообщения хранятся в порядке возрастания курсора.
   * Если в канале уже существует сообщение(я) с указаным в m курсором,
   * то m добавляется за ним(и)
   */
  void publish_(time_t now, stored_message::ptr m);

  /// @brief обновляет все сообшения с обновлением времени создания
  void update_(time_t now, stored_message::ptr m);

  /// @brief обновляет все сообшения без обновления времени создания
  void modify_(time_t now, stored_message::ptr m );

  /// @brief Удаляет все сообщения по identity
  //void _remove( time_t now, stored_message::ptr m, bool publish_flag /*= true*/ );
  void remove_( time_t now, stored_message::ptr m);
  void remove_( time_t now, const message& m);

  /// @brief
  void replace_(time_t now, stored_message::ptr m );
  
private:
  queue_type _queue;
  
  /// флаг того, что время жизни всех сообщений канала упорядоченно 
  bool _sequenced_flag;
  time_t _last_remove_death;
  mutable size_t _removed;

};

}}


