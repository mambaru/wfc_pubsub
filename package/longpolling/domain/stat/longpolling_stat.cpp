//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "longpolling_stat.hpp"
#include <wfc/statistics/statistics.hpp>

namespace wfc{ namespace pubsub{

namespace {

void meter_set(wfc::value_meter& m, size_t value)
{
  m.create(static_cast<wrtstat::value_type>(value), 0ul);
}

} // namespace

void longpolling_stat::start(wfc::statistics::statistics& st)
{
  _meters = {
    // 0. Все отправленные сообщения
    st.create_value_meter("sended_messages"),
    // 1. Сообщения ожидающие подверждения о доставке
    st.create_value_meter("wait_messages"),
    // 2. Сообщения хранимые в агентах
    st.create_value_meter("stored_messages"),
    // 3. Удалено устаревших сообщений
    st.create_value_meter("remove_death"),
    // 4. все каналы со всех агентов включая дубликаты
    st.create_value_meter("active_channels"),
    // 5. Активные агенты
    st.create_value_meter("active_agents"),
    // 6. Удаленные агенты
    st.create_value_meter("deleted_agents"),
    // 7. Сообщения хранимые в агентах
    st.create_value_meter("hub_stored_messages"),
    // 8. все каналы со всех агентов включая дубликаты
    st.create_value_meter("hub_active_channels"),
    // 9. Удалено устаревших сообщений из общего хаба
    st.create_value_meter("hub_remove_death"),
    // 10.
    st.create_value_meter("counter_map"),
    // 11.
    st.create_value_meter("wait_subscribe"),
    // 12.
    st.create_value_meter("wait_describe"),
    // 13.
    st.create_value_meter("describe_delay"),
    // 14.
    st.create_value_meter("inflight_subscribe"),
    // 15.
    st.create_value_meter("inflight_describe"),
    // 16.
    st.create_value_meter("describe_count"),
    // 17.
    st.create_value_meter("subscribe_count"),
    // 18.
    st.create_value_meter("active_agents_uc"),
    // 19.
    st.create_value_meter("dead_describe"),
    // 20. Отказы авторизации (всего за fire-тик)
    st.create_value_meter("auth_errors"),
    // 21.
    st.create_value_meter("auth_empty"),
    // 22.
    st.create_value_meter("auth_expired"),
    // 23.
    st.create_value_meter("auth_wrong_sid"),
    // 24.
    st.create_value_meter("auth_no_auth"),
    // 25.
    st.create_value_meter("auth_no_agent"),
    // 26.
    st.create_value_meter("auth_oid_mismatch"),
    // 27.
    st.create_value_meter("auth_anonymous_denied"),
    // 28. Уникальные (oid,sid) с залогированным create fail
    st.create_value_meter("auth_fail_sid_oid"),
    // 29. Агенты без oid
    st.create_value_meter("auth_unauthorized_agents"),
    // 30. Размер session store
    st.create_value_meter("auth_sessions"),
    // 31. Снято из fail-лога при clear_fail_log (delta)
    st.create_value_meter("auth_fail_erased")
  };
}

bool longpolling_stat::enabled() const
{
  return !_meters.empty();
}

void longpolling_stat::write(
  const fire_stat& cur,
  size_t describe_count,
  size_t subscribe_count,
  const auth_error_stat& auth_st)
{
  if ( _meters.empty() )
    return;

  meter_set(_meters.at(0), cur.sended_messages);
  meter_set(_meters.at(1), cur.wait_messages);
  meter_set(_meters.at(2), cur.stored_messages);
  meter_set(_meters.at(3), cur.remove_death);
  meter_set(_meters.at(4), cur.active_channels);
  meter_set(_meters.at(5), cur.active_agents);
  meter_set(_meters.at(6), cur.deleted_agents);
  meter_set(_meters.at(7), cur.hub_stored_messages);
  meter_set(_meters.at(8), cur.hub_active_channels);
  meter_set(_meters.at(9), cur.hub_remove_death);
  meter_set(_meters.at(10), cur.hub_queue.counter_map);
  meter_set(_meters.at(11), cur.hub_queue.wait_subscribe);
  meter_set(_meters.at(12), cur.hub_queue.wait_describe);
  meter_set(_meters.at(13), cur.hub_queue.describe_delay);
  meter_set(_meters.at(14), cur.hub_queue.inflight_subscribe);
  meter_set(_meters.at(15), cur.hub_queue.inflight_describe);
  meter_set(_meters.at(16), describe_count);
  meter_set(_meters.at(17), subscribe_count);
  meter_set(_meters.at(18), cur.active_agents_uc);
  meter_set(_meters.at(19), cur.dead_describe);

  meter_set(_meters.at(20), auth_st.total);
  meter_set(_meters.at(21), auth_st.empty);
  meter_set(_meters.at(22), auth_st.expired);
  meter_set(_meters.at(23), auth_st.wrong_sid);
  meter_set(_meters.at(24), auth_st.no_auth);
  meter_set(_meters.at(25), auth_st.no_agent);
  meter_set(_meters.at(26), auth_st.oid_mismatch);
  meter_set(_meters.at(27), auth_st.anonymous_denied);
  meter_set(_meters.at(28), auth_st.fail_sid_oid);
  meter_set(_meters.at(29), auth_st.unauthorized_agents);
  meter_set(_meters.at(30), auth_st.sessions);
  meter_set(_meters.at(31), auth_st.fail_erased);
}

}}
