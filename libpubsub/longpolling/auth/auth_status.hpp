//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

namespace wfc{ namespace pubsub{

enum class auth_status
{
  ok = 0,
  empty,             // oid==0 или sid пустой (не анонимный вход)
  expired,           // пара oid/sid была в store, но TTL истёк
  wrong_sid,         // у oid есть другие sid, этот не совпал / не приходил
  no_auth,           // по oid в store ничего нет (comet.auth не пришёл)
  no_agent,          // агент с uuid не найден
  oid_mismatch,      // oid запроса не совпадает с oid агента
  anonymous_denied   // аноним запрещён (allow_anonymous=false)
};

inline const char* auth_status_str(auth_status st)
{
  switch ( st )
  {
  case auth_status::ok: return "ok";
  case auth_status::empty: return "empty";
  case auth_status::expired: return "expired";
  case auth_status::wrong_sid: return "wrong_sid";
  case auth_status::no_auth: return "no_auth";
  case auth_status::no_agent: return "no_agent";
  case auth_status::oid_mismatch: return "oid_mismatch";
  case auth_status::anonymous_denied: return "anonymous_denied";
  default: return "unknown";
  }
}

}}
