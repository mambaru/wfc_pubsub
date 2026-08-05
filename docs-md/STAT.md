# Статистика longpolling

Метрики fire-тика longpolling-домена (wrtstat value meters). Включаются только если у domain-объекта есть `get_statistics()` (WFC, `-DWFC_ENABLE_STAT=ON` и статистика не отключена в runtime).

## Код

| Компонент | Путь | Роль |
|-----------|------|------|
| `longpolling_stat` | `package/longpolling/domain/stat/` | Создание meters и запись значений на каждом fire |
| `fire_stat` | `libpubsub/longpolling/fire_stat.hpp` | Снимок агентов / hub после `longpolling::fire` |
| `hub_queue_stat` | `libpubsub/longpolling/hub_queue_stat.hpp` | Очереди subscribe/describe к hub |
| `auth_error_stat` | `libpubsub/longpolling/auth/auth_error_stat.hpp` | Отказы auth и gauge сессий |
| `longpolling_auth` | `libpubsub/longpolling/auth/` | Проверки oid/sid, store, счётчики |

Поток:

1. `longpolling_domain::start()` → при наличии статистики `_stat.start(*st)` создаёт meters
2. Таймер `fire_timer` → `longpolling_fire_()`
3. `make_descriptions_` / `make_subscriptions_` → накопление `describe_count` / `subscribe_count`
4. `_longpolling->fire(&cur_stat)` → заполняет `fire_stat`
5. `_auth.take_error_stat(*_longpolling)` → дельты отказов + текущие gauge
6. `_stat.write(cur_stat, describe_count, subscribe_count, auth_st)`

Имена meters без префикса domain; префикс добавляет слой WFC statistics при агрегации/отправке.

## Типы значений

| Тип | Смысл | Примеры |
|-----|--------|---------|
| **delta** | Значение за интервал fire / с прошлого `take` | `sended_messages`, `auth_errors`, `remove_death` |
| **gauge** | Текущее состояние на момент fire | `active_agents`, `wait_subscribe`, `auth_sessions` |
| **period** | Накопление с прошлого `fire_log_s`, затем сброс в логе | `subscribe_count`, `describe_count` |

`subscribe_count` / `describe_count` в meters — накопленное число каналов, ушедших в RPC с начала периода `fire_log_s` (сбрасываются вместе с drain-логами).

---

## Метрики агентов и доставки (0–9, 18–19)

Источник: `fire_stat` из `longpolling::fire`.

| # | Meter | Тип | Смысл |
|---|-------|-----|--------|
| 0 | `sended_messages` | delta | Сообщения, отданные в longpoll handlers за тик |
| 1 | `wait_messages` | gauge | Сообщения, ждущие confirm доставки |
| 2 | `stored_messages` | gauge | Сообщения, хранимые в агентах |
| 3 | `remove_death` | delta | Удалено устаревших сообщений в агентах |
| 4 | `active_channels` | gauge | Каналы по всем агентам (с дубликатами) |
| 5 | `active_agents` | gauge | Живые агенты |
| 6 | `deleted_agents` | delta | Удалённые за тик агенты |
| 7 | `hub_stored_messages` | gauge | Сообщения в `_common_hub` |
| 8 | `hub_active_channels` | gauge | Каналы в `_common_hub` |
| 9 | `hub_remove_death` | delta | Удалено устаревших из `_common_hub` |
| 18 | `active_agents_uc` | gauge | Агенты с уникальными каналами (карта каналов) |
| 19 | `dead_describe` | delta | Отписки для удалённых агентов |

---

## Очереди hub subscribe/describe (10–17)

Источник: `fire_stat.hub_queue` (`hub_queue_stat`) и счётчики drain в domain.

| # | Meter | Тип | Смысл |
|---|-------|-----|--------|
| 10 | `counter_map` | gauge | Размер ref-count карты подписок на hub |
| 11 | `wait_subscribe` | gauge | Каналы в очереди на subscribe к hub |
| 12 | `wait_describe` | gauge | Каналы в очереди на describe |
| 13 | `describe_delay` | gauge | Каналы в delayed-describe (`describe_delay_s`) |
| 14 | `inflight_subscribe` | gauge | Subscribe RPC в полёте (manager) |
| 15 | `inflight_describe` | gauge | Describe RPC в полёте (manager) |
| 16 | `describe_count` | period | Каналов отправлено describe за период `fire_log_s` |
| 17 | `subscribe_count` | period | Каналов отправлено subscribe за период `fire_log_s` |

Связанные опции: `subscribe_batch`, `subscribe_inflight_max`, `describe_inflight_max`, `inflight_timeout_s`, `fire_log_s`. Drain-логи (`Subscribe drain period…`) пишутся в longpoll-лог, не в wrtstat.

---

## Авторизация (20–30)

Источник: `longpolling_auth::note_unauthorized` / `take_error_stat`.

### Дельты отказов (сбрасываются при каждом `write`)

Считается каждый отказ проверки (`create` / `open` / `close` / `longpoll` / `publish`), в том числе при `auth.suspend` (клиенту отказ не отдаётся, счётчик растёт).

| # | Meter | `auth_status` / смысл |
|---|-------|------------------------|
| 20 | `auth_errors` | Сумма всех отказов за fire-тик |
| 21 | `auth_empty` | `empty` — oid==0 или пустой sid (не анонимный вход) |
| 22 | `auth_expired` | `expired` — пара была в store, TTL истёк |
| 23 | `auth_wrong_sid` | `wrong_sid` — у oid другие sid, этот не совпал |
| 24 | `auth_no_auth` | `no_auth` — по oid в store ничего нет (`comet.auth` не приходил) |
| 25 | `auth_no_agent` | `no_agent` — агент с uuid не найден |
| 26 | `auth_oid_mismatch` | `oid_mismatch` — oid запроса ≠ oid агента |
| 27 | `auth_anonymous_denied` | `anonymous_denied` — аноним запрещён |

### Gauge

| # | Meter | Смысл |
|---|-------|--------|
| 28 | `auth_fail_sid_oid` | Размер set уникальных `(oid,sid)` с залогированным `create: unauthorized`. При успешном `create` или `comet.auth` для oid удаляются **все** sid этого oid |
| 29 | `auth_unauthorized_agents` | Агенты без oid (`count_agents_without_oid`) |
| 30 | `auth_sessions` | Число сессий в `session_store` |

### Восстановление после fail

| # | Meter | Тип | Смысл |
|---|-------|-----|--------|
| 31 | `auth_fail_erased` | delta | Сколько `(oid,sid)` снято через `clear_fail_log` за fire-тик |

Те же gauge периодически попадают в WARNING-лог `Auth:` / `Auth grace:` (`fire_log`).

---

## Когда метрик нет

- Сборка без `WFC_ENABLE_STAT`
- `global()->disable_statistics`
- Domain ещё не сконфигурирован / `get_statistics() == nullptr`

Тогда `_stat.enabled() == false`, `write` не вызывается. Счётчики `note_unauthorized` при этом всё равно инкрементируются; сброс происходит только при `take_error_stat` внутри `write`.

---

## Отладка без wrtstat

| Инструмент | Что смотреть |
|------------|--------------|
| `get_hub_queue_stat()` | `counter_map`, wait/inflight subscribe/describe |
| Логи drain | backlog, throttle по `*_inflight_max` |
| `Auth:` / `Auth grace:` | sessions, unauthorized_agents, fail_sid_oid |
| `reason=` в unauthorized-логах | причина отказа (см. [DEVELOPER.md](DEVELOPER.md) — Auth) |
