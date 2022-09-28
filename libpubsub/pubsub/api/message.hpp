//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2013, 2014, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <pubsub/api/types.hpp>
#include <pubsub/api/actions.hpp>
#include <fas/system/memory.hpp>

namespace wfc{ namespace pubsub{

struct message
{
  actions action = actions::publish;

  size_t limit = 0;

  /** Время регистрации. устанавлявается сервером в момент поступления события */
  mutable time_t birthtime = 0;

  /** Время жизни в секундах или время смерти в unix timespan
    */
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

  bool persistent = false;
};


}}

