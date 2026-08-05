#pragma once

#include <string>
#include <message_queue/types.hpp>

namespace wfc{ namespace pubsub{

struct agent_options
{
  std::string uuid;

  // Идентификатор пользователя, привязанный к агенту при авторизации
  user_id_t oid = 0;

  // Идентификатор пользователя для приватных сообщений (message.key)
  key_t key = 0;

  agent_options() = default;
  explicit agent_options(std::string uuid_)
    : uuid(std::move(uuid_))
  {}

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

