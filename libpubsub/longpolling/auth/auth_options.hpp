#pragma once

#include <string>
#include <message_queue/types.hpp>

namespace wfc{ namespace pubsub{

struct auth_options
{
  // Отключить проверку авторизации (oid/sid)
  bool disabled = false;

  // Авторизация включена (канал, store, проверки в логах), но отказ клиенту не отдаётся
  bool suspend = false;

  // Канал, куда бэкенд публикует пары oid/sid
  std::string channel;

  // TTL сессии в секундах (переопределяет lifetime из сообщения, если > 0)
  time_t ttl_s = 3600;

  // Максимум одновременных sid на один oid. 0 — без ограничения.
  // При превышении удаляется сессия с наименьшим expire (самый старый sid).
  size_t max_sessions_per_oid = 100;

  // true — без oid/sid можно работать только с анонимным агентом (key=0).
  // false — без oid/sid все операции отклоняются.
  // Если oid/sid переданы — проверяется сессия и соответствие агенту.
  bool allow_anonymous = false;

  // Пока grace_period_s не истёк, пары oid/sid, ещё не попавшие в store (comet.auth),
  // принимаются. Если пара уже есть в store — проверка как обычно.
  time_t grace_period_s = 0;
};

}}
