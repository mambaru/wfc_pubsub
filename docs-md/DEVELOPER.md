# Руководство разработчика: wfc_pubsub

Демон pub/sub с longpolling (comet) на базе WFC. Репозиторий состоит из двух слоёв:

| Слой | Путь | Назначение |
|------|------|------------|
| **libpubsub** | `libpubsub/` | Библиотека: очереди, hub, агенты longpoll |
| **package** | `package/` | WFC-модули: домены, JSON-RPC-сервисы, HTTP-фасад |

Зависимости: `message_queue` → `pubsub` → `longpolling` (внутри libpubsub); в runtime — цепочка доменов WFC.

Документация: [STAT.md](STAT.md) — метрики fire / auth (wrtstat).

---

## Архитектура runtime

Типичная конфигурация comet-клиентов:

```
Клиент (HTTP POST)
    ↓
http_domain          — разбор HTTP, merge query → JSON-RPC
    ↓
longpolling-service  — JSON-RPC-движок (wjrpc)
    ↓
longpolling_domain   — агенты, auth, подписки на hub
    ↓
pubsub_domain        — центральный hub (память + RocksDB)
```

Параллельно доступны:

- **pubsub-service** — JSON-RPC к hub напрямую (без longpoll)
- **pubsub-gateway** — исходящий клиент к удалённому hub

Пакет регистрируется в `pubsub_package.cpp`:

```cpp
module_list< build_info, http_module, pubsub_module, longpolling_module >
```

---

## libpubsub

### message_queue

Низкоуровневое in-memory хранилище сообщений по каналам.

| Компонент | Файлы | Роль |
|-----------|-------|------|
| `message` | `message.hpp` | Сообщение: `action`, `cursor`, `content`, `lifetime`, `limit`, `key` (получатель, см. ниже), `persistent` |
| `message_queue` | `message_queue.*` | Очередь одного канала, TTL, действия `publish`/`modify`/`update`/`replace`/`remove` |
| `channel_map` | `channel_map.*` | `map<channel, message_queue>` |
| `topic` | `topic.hpp` | `message` + имя канала — единица доставки клиенту |

Приоритетные сообщения: `limit == 0 && lifetime == 0` (`message::is_prio()`).

### pubsub

Центральный hub: публикация, подписка, describe, чтение истории.

| Компонент | Роль |
|-----------|------|
| `pubsub` | Один шард hub: handlers, in-memory cache, RocksDB |
| `pubsub_mt` | Шардирование по `hash_size`, отдельный mutex на шард |
| `multi_rocksdb` | Пул RocksDB по TTL-интервалам; mutex на map DB и кэш имён каналов |
| `rocksdb` | Персистентные сообщения (`persistent` или `persistent_default`) |
| `rocksdb_factory` | Open/create DB; `create_if_missing` всегда включён после configure |

Основной поток **publish**:

1. Применить `default_lifetime` / `default_limit`
2. Записать в RocksDB (если persistent)
3. Положить в `channel_map` (если cache не отключён)
4. Вызвать handlers подписчиков канала

**subscribe** регистрирует handler; при необходимости отдаёт историю из RocksDB и cache.

**`multi_rocksdb` (персистентность):**

- `configure(channels_cache, opt)` — `channels_cache == true`, когда `persistent_default == false` (кэш имён каналов, в которые был `push`)
- `get_messages` — снимок указателей DB под mutex, чтение вне lock (нет гонки с `configure`/`close`)
- `has(channel)` — при включённом кэше: был ли `push` в этот канал в текущем процессе; после overflow (`channels_cache_max`, 1e6) кэш сбрасывается и `has` больше не отвечает «нет» (только «неизвестно» → `true`)
- `content` в RocksDB — **raw JSON** (`wjson::raw_value`), не произвольная строка

Опции (`pubsub_options`):

| Поле | По умолчанию | Смысл |
|------|--------------|-------|
| `persistent_default` | false | Все сообщения persistent |
| `rocksdb_disabled` | false | Только in-memory |
| `cache_disabled` | false | Не кэшировать в памяти |
| `default_lifetime` | 60 | TTL по умолчанию (с) |
| `default_limit` | 10 | Лимит очереди по умолчанию |
| `hash_size` | 64 | Число шардов `pubsub_mt` |

### longpolling

Comet-слой: агенты клиентов, буфер hub, координация subscribe/describe к удалённому pubsub.

| Компонент | Роль |
|-----------|------|
| `agent` | Один клиент (`uuid`): каналы, longpoll handler, `fire()` / `confirm()` |
| `agent_map` / `agent_map_mt` | Карта агентов, fan-out `push(channel)`; longpoll callback **вне** shard-lock |
| `subscribe_manager` | Ref-count подписок на hub; очереди wait/inflight; retry по `inflight_timeout_s` |
| `wait_list` | Очередь каналов (subscribe/describe), вынесена из manager |
| `hub_queue_stat` / `fire_stat` | Снимки очередей hub и статистики `fire` |
| `hub_rpc_inflight` | Лимит одновременных subscribe/describe RPC domain→hub |
| `longpolling` | Фасад: `create`/`open`/`close`/`longpoll`/`push`/`fire` |
| `auth/session_store` | TTL-таблица `oid`/`sid`, индексы по expire и лимит sid на oid |
| `auth/longpolling_auth` | Проверки, grace, comet.auth, счётчики отказов |
| `auth/auth_options` | Опции auth (JSON + struct) |
| `publish_options` | Разрешение клиентского publish по префиксам каналов |

**Жизненный цикл клиента:**

1. `create(agent_options)` — новый агент с UUID
2. `open(agent_params, channels)` — открыть каналы; при первом подписчике канал уходит в очередь subscribe к hub; история из `_common_hub` копируется агенту **после** unlock hub-mutex (нет AB–BA с `fire`: open не держит hub→shard, fire — shard→hub)
3. `longpoll(params, handler)` — зарегистрировать callback; предыдущий waiter получает пустой список **вне** lock
4. `fire(stat)` (периодически) — накопить `pending_longpoll`, вызвать handlers вне shard-lock; `_wait_list` чистит только `confirm()`
5. Нет агента / отказ auth — domain всегда вызывает `cb(nullptr)` (без вечного hang)

**Hub → клиенты:**

1. Domain забирает каналы из `pop_for_subscribe` / `pop_for_describe` → RPC к pubsub (лимиты `subscribe_inflight_max` / `describe_inflight_max`)
2. Ответ/notify → `longpolling::push(channel, msg)` → `_common_hub` + fan-out агентам
3. Следующий `fire()` отдаёт сообщения в longpoll handler
4. При таймауте RPC / `stop` / смене UUID hub — domain сбрасывает/восстанавливает inflight-счётчики (deadlines), manager — `inflight_*` по `inflight_timeout_s`

Опции (`longpolling_options`):

| Поле | Смысл |
|------|-------|
| `hash_size` | Шарды `agent_map_mt` |
| `describe_delay_s` | Задержка отписки от hub после закрытия канала |
| `inflight_timeout_s` | Таймаут ожидания ответа hub на subscribe/describe; по истечении — повтор |
| `describe_suspend` | Отключить describe (отладка) |

Опции агента (`agent_options` / `agent_params`):

| Поле | Смысл |
|------|-------|
| `uuid` | Идентификатор агента |
| `oid` | Пользователь (при auth) |
| `sid` | Идентификатор сессии |
| `key` | Ключ получателя приватных сообщений (см. [Приватные сообщения](#приватные-сообщения)) |
| `agent_lifetime` | TTL агента без запросов (с) |
| `longpoll_timeout` | Таймаут ожидания longpoll (с) |
| `longpoll_limit` | Макс. сообщений за один longpoll |
| `confirm` | Подтверждение доставки предыдущего ответа |

### Приватные сообщения

Поле `message.key` задаёт адресата сообщения внутри общего канала:

| `message.key` | Смысл |
|---------------|-------|
| `0` | Публичное сообщение — доставляется всем агентам, подписанным на канал |
| `≠ 0` | Приватное — только агенту с `agent_options.key == message.key` |

Проверка в `message::for_recipient(m, recipient_key)`:

```cpp
return m.key == 0 || m.key == recipient_key;
```

**Где фильтруется.** Hub (`pubsub`) хранит все сообщения без разделения по получателям. Фильтрация — при доставке агенту в `agent::push()` (первая проверка в методе). Fan-out `agent_map::push(channel)` вызывает `push` у каждого подписчика канала; чужие приватные сообщения отбрасываются внутри агента.

**Ключ агента.** В `agent_options.key` хранится идентификатор пользователя для приёма приватных сообщений. Его можно передать в `create` (JSON-RPC / query `key`). При включённой авторизации, если клиент не указал `key`, но передал `oid`, domain подставляет `key = oid`:

```cpp
if ( ao.key == 0 && req->oid != 0 )
  ao.key = static_cast<key_t>(req->oid);
```

Так приватное сообщение с `key=1001` попадёт агенту пользователя `1001` без явного `key` в запросе `create`.

**Публикация.** Поле `key` передаётся в теле сообщения при `publish` (через `longpolling-service` или `pubsub-service`):

```json
{
  "method": "publish",
  "params": {
    "channel": "orders",
    "message": {
      "identity": "order-42",
      "key": 1001,
      "content": { "status": "ready" }
    }
  },
  "id": 1
}
```

Сообщение с `"key": 0` (или без поля) увидят все подписчики канала `orders`; с `"key": 1001` — только агент с `key=1001`.

**HTTP.** Параметр `key` в query merge попадает в `create` / `open` (см. `jsonrpc_query_merge`).

**Тесты.** `libpubsub/tests/agent_suite.cpp` — юнит `agent_msgkey` (два агента на одном канале, публичное + приватные сообщения).

### Сборка libpubsub

```
libpubsub/
├── message_queue/   → target message_queue
├── pubsub/          → target pubsub (в т.ч. rocksdb/)
├── longpolling/     → target longpolling (+ auth/)
└── tests/           → target test_pubsub (если BUILD_TESTING)
```

Заголовки: `<message_queue/...>`, `<pubsub/...>`, `<longpolling/...>`, `<longpolling/auth/...>`.

---

## package (WFC)

### Модули

| WFC-имя | Тип | Интерфейс |
|---------|-----|-----------|
| `http` | `http_domain` | `ihttp` |
| `pubsub` | `pubsub_domain` | `ipubsub` |
| `longpolling` | `longpolling_domain` | `ilongpolling` |
| `pubsub-service` | JSON-RPC service | → `ipubsub` |
| `longpolling-service` | JSON-RPC service | → `ilongpolling` |
| `pubsub-gateway` | JSON-RPC gateway | удалённый hub |

Каждый домен — `wfc::instance<Domain>` + `*_config_json` для десериализации конфига. Связь между доменами — поле `target` / `target_name` (имя в registry WFC).

### Жизненный цикл домена

| Фаза WFC | Хук домена | Типичные действия |
|----------|------------|-------------------|
| configure | `configure()` | Создание движков (`pubsub_mt`, `longpolling`) |
| initialize | `initialize()` | `get_target<>()` — привязка к другим доменам |
| start | `start()` | Таймеры, подписки, статистика |
| stop | `stop()` | Остановка таймеров, закрытие RocksDB |

### pubsub_domain

Конфиг (`pubsub_config` = `pubsub_options` + `rocksdb_options` + extras):

```json
{
  "name": "pubsub1",
  "enabled": true,
  "rocksdb_disabled": true,
  "path": "/var/pubsub/rocksdb",
  "ini": "/etc/pubsub/rocksdb.ini",
  "control_s": "60s"
}
```

JSON-RPC методы (`pubsub-service`): `publish`, `subscribe`, `describe`, `get_messages`, `ping`.

### longpolling_domain

Конфиг (`longpolling_config`):

```json
{
  "name": "longpolling1",
  "enabled": true,
  "target": "pubsub1",
  "fire_timer_ms": "1000ms",
  "fire_log_s": "10s",
  "ping_timer_ms": "1000ms",
  "subscribe_batch": 100,
  "subscribe_inflight_max": 32,
  "describe_inflight_max": 32,
  "describe_delay_s": "5s",
  "inflight_timeout_s": "30s",
  "auth": {
    "disabled": false,
    "suspend": false,
    "channel": "auth.sessions",
    "ttl_s": 3600,
    "max_sessions_per_oid": 100,
    "grace_period_s": 30,
    "allow_anonymous": false
  },
  "publish": {
    "enabled": true,
    "channel_prefixes": ["client.", "notify."]
  }
}
```

Доп. поля domain-конфига:

| Поле | По умолчанию | Смысл |
|------|--------------|-------|
| `subscribe_batch` | 100 | Каналов в одном RPC subscribe/describe |
| `subscribe_inflight_max` | 32 | Макс. одновременных subscribe к hub (`0` — без лимита) |
| `describe_inflight_max` | 32 | То же для describe |

JSON-RPC методы (`longpolling-service`): `create`, `open`, `close`, `longpoll`, `publish`.

**Ограничение publish клиентом.** Блок `publish` управляет методом `publish` через longpolling-service (клиентский JSON-RPC / HTTP). По умолчанию `publish.enabled = false` — **все клиенты запрещены** публиковать в hub. При `enabled: true` действует фильтр `channel_prefixes`: канал разрешён, если имя начинается с одного из префиксов; `["*"]` — любые каналы. Пустой `channel_prefixes` при `enabled: true` — publish всё равно запрещён. Проверка в `longpolling_domain::publish` до пересылки в hub; при нарушении — отказ (`cb(nullptr)`). На внутренний `notify` от hub ограничение не распространяется.

Примеры:

| `publish.enabled` | `publish.channel_prefixes` | Поведение |
|-------------------|----------------------------|-----------|
| `false` или не задано | любые | publish запрещён |
| `true` | `[]` | publish запрещён |
| `true` | `["*"]` | любые каналы |
| `true` | `["client.", "notify."]` | только каналы с этими префиксами |

**Таймеры в `start()`:**

- `fire_timer` — `recover_inflight_deadlines_`, `make_subscriptions_`, `make_descriptions_`, `longpolling::fire` (через `weak_ptr`)
- `ping_timer` — проверка UUID hub; при смене UUID — `resubscribe()` + сброс domain-inflight

**Auth** (при `!auth.disabled` и непустом `auth.channel`; код в `libpubsub/longpolling/auth/` — `longpolling_auth`, `session_store`):

1. При старте: `ensure_subscribed(auth.channel)` → subscribe к hub на fire-тике
2. Сообщения в auth-канале → `session_store` (`{"oid":1001,"sid":"..."}`)
3. Проверка `oid`/`sid` в `create`/`open`/`close`/`longpoll`/`publish`
4. `grace_period_s` — после перезапуска проверка отложена; на каждом fire — WARNING с числом сессий и неавторизованных агентов
5. `max_sessions_per_oid` — лимит активных sid на oid (при превышении вытесняется самый старый по expire); `0` — без лимита
6. Логи `create/open/...: unauthorized` содержат `reason=`:
   - `no_auth` — по oid в store ничего нет (`comet.auth` не приходил)
   - `wrong_sid` — у oid уже есть другие sid, этот не совпал
   - `expired` — пара была, но TTL истёк
   - также: `empty`, `anonymous_denied`, `no_agent`, `oid_mismatch`
   Логи `create: unauthorized` дедуплицируются по паре `(oid, sid)`
7. `auth.suspend` — проверки и логи как обычно, но клиенту **не** отдаётся отказ (`cb(nullptr)`); удобно для обкатки auth без блокировки пользователей

### http_domain

Конфиг (`http_config`):

```json
{
  "name": "http1",
  "enabled": true,
  "target": "longpolling-service1",
  "jsonrpc_path": "/comet"
}
```

Поведение:

- Парсинг HTTP (Boost.Beast), маршрутизация по `jsonrpc_path`
- Merge query → `params`: `oid`, `sid`, `uuid`, `key`, `confirm`, `channels` (query перекрывает body)
- Пустое body + query → сборка JSON-RPC из query (GET-совместимость)
- Ответы — HTTP 200 `application/json`

Особые случаи (совместимость comet):

- `id == -1` и `method` ∈ {`create`, `subscribe`} — встречный вызов (counter-call)
- `id == -1` иначе — notify, пустой HTTP-ответ

### Пример полной связки (как в тестах)

```cpp
pubsub_conf.name = "pubsub1";
pubsub_conf.rocksdb_disabled = true;

longpolling_conf.name = "longpolling1";
longpolling_conf.target = "pubsub1";

lp_serv_conf.name = "longpolling-service1";
lp_serv_conf.target_name = "longpolling1";

http_conf.name = "http1";
http_conf.target = "longpolling-service1";
http_conf.jsonrpc_path = "/comet";
```

---

## JSON и API

Сериализация — **wjson** (`*_json.hpp`). Строгий режим в конфигах доменов (`wjson::strict_mode`).

Типы запросов/ответов: `package/*/api/*.hpp` + `*_json.hpp`.

Клиентский longpoll через HTTP:

```http
POST /comet?oid=1001&sid=sess1&uuid=<uuid> HTTP/1.1
Content-Type: application/json

{"method":"longpoll","params":{},"id":2}
```

---

## Сборка и тесты

```bash
make static          # релиз, статическая линковка
make paranoid        # тесты + максимум предупреждений (-Werror)
make tests           # cmake BUILD_TESTING + ctest
```

| Target | Содержимое |
|--------|------------|
| `test_pubsub` | libpubsub: message, agent / agent_msgkey, longpolling, session_store, **rocksdb** (`get_messages`, multi TTL, `has`) |
| `test_package` | package: pubsub, http, longpoll chain, auth, publish prefix, open flow |

Юниты RocksDB пишут во временный каталог (`/tmp/wfc_pubsub_rocksdb_tests/...`) и удаляют его после теста. `content` в сообщениях — валидный raw JSON (например `"\"c1\""`).

Интеграционные тесты цепочки: `tests/package_suite.cpp`, `tests/longpoll_suite.cpp` (auth grace, publish prefix, open flow).

---

## Структура каталогов (кратко)

```
wfc_pubsub/
├── libpubsub/
│   ├── message_queue/     # очереди, channel_map
│   ├── pubsub/            # hub, pubsub_mt, rocksdb/
│   ├── longpolling/       # agent, subscribe_manager, wait_list, fire_stat, hub_rpc_inflight
│   │   └── auth/          # session_store, longpolling_auth, auth_status, auth_error_stat
│   └── tests/             # test_pubsub (+ rocksdb_suite)
├── package/
│   ├── http/domain/       # http_domain, jsonrpc_query_merge
│   ├── pubsub/            # domain, service, gateway, api/
│   ├── longpolling/       # domain, service, api/
│   │   └── domain/
│   │       └── stat/      # longpolling_stat (wrtstat meters)
│   └── pubsub_package.cpp
├── tests/                 # test_package, longpoll_suite
├── docs-md/               # DEVELOPER.md, STAT.md
└── configurations/        # примеры конфигов деплоя (submodule)
```

---

## Расширение

**Новый JSON-RPC метод longpolling:**

1. Добавить `request`/`response` в `package/longpolling/api/`
2. Добавить `*_json.hpp`
3. Зарегистрировать в `longpolling_service_method_list`
4. Реализовать в `longpolling_domain` (или делегировать в `longpolling`)

**Новое поле в merge query (HTTP):**

1. `package/http/domain/detail/jsonrpc_query_merge.cpp` — `overlay_from_query_`, `merge_into_body`

**Отладка подписок hub:**

- Логи `subscribe_manager` / drain: таймауты inflight, rollback, describe delay, throttle по `*_inflight_max`
- `get_hub_queue_stat()` — размеры `counter_map`, wait/inflight очередей
- Метрики fire / auth: [STAT.md](STAT.md) (`domain/stat/longpolling_stat`, wrtstat)

---

## Связь со старым cometd

Новый демон воспроизводит ключевые контракты comet:

- HTTP POST + query (`oid`, `sid`, `uuid`)
- JSON-RPC методы longpolling
- Auth через hub-канал с `oid`/`sid`
- `grace_period_s` при перезапуске (клиенты не отваливаются до прихода сессий)
