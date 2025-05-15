//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2013, 2014, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <message_queue/types.hpp>
#include <message_queue/actions.hpp>
#include <fas/system/memory.hpp>

namespace wfc{ namespace pubsub{

struct message
{
  actions action = actions::publish;

  size_t limit = 0;

  /** Время жизни в секундах или время смерти в unix timespan */
  time_t lifetime = 0;

  /** некоторое числовое значение, по которому может выгружаться информация из канала
   *  (выгружаются все у которых cursor >= заданного значения )
   *  а также сортировка
   */
  cursor_t cursor = 1;

  /** Некоторый идентификатор по которому происходит обновление или удаление сообщения из канала. */
  identity_t identity = "";

  /** контент сообщения. */
  content_ptr content = nullptr;

  /** ключ доступа для client-side*/
  key_t key = 0;

  /** Флаг персистентности */
  bool persistent = false;

  /** Время регистрации. устанавлявается сервером в момент поступления события */
  mutable time_t birthtime = 0;

  static message copy(const message& m)
  {
    message newm;
    newm.action = m.action;
    newm.limit = m.limit;
    newm.birthtime = m.birthtime;
    newm.lifetime = m.lifetime;
    newm.cursor = m.cursor;
    newm.identity = m.identity;
    newm.persistent = m.persistent;
    newm.key = m.key;
    if ( m.content!=nullptr )
      newm.content = std::make_unique<content_t>(*m.content);
    return newm;
  }

  /**
   * @brief Приоритетное сообщение. Доставляеться пользователю "как есть" без сохранения в канал.
   * Для таких сообщений создается отдельная очередь, со своими ограничениями
   */
  static bool is_prio(const message& m)
  {
    return m.action == actions::publish && m.limit==0 && m.lifetime==0;
  };
};


}}

