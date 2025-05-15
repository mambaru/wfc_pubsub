#pragma once

#include <string>
#include <message_queue/types.hpp>

namespace wfc{ namespace pubsub{

struct agent_options
{
  std::string uuid;

  // ключ для приватных сообщеий (id-пользователя)
  key_t key = 0;

  // Время жизни агента после последнего запроса от клиента
  time_t agent_lifetime = 3600;

  // Таймаут longpoll обработчика если не поступают сообщения
  time_t longpoll_timeout = 600;

  // Ограничение на отправку на один запрос
  size_t longpoll_limit = 100;

  // Ограничение на очередь приоритетных сообщений (message::is_prio)
  size_t prio_limit = 100;
};

}}

