# wfc_pubsub

Демон pub/sub с longpolling (comet) на базе WFC.

## Быстрый старт

```bash
make static          # релиз
make paranoid        # тесты + максимум предупреждений
./build/bin/tests/test_package
```

## Документация

**[docs-md/DEVELOPER.md](docs-md/DEVELOPER.md)** — руководство разработчика:

- архитектура `libpubsub` (message_queue → pubsub → longpolling)
- WFC-модули `package` (http, pubsub, longpolling, сервисы)
- конфигурация, JSON-RPC, потоки данных
- auth, HTTP merge query, приватные сообщения (`message.key`), сборка и тесты

## Слои проекта

| Путь | Назначение |
|------|------------|
| `libpubsub/` | Библиотека: hub, агенты, очереди |
| `package/` | Домены WFC, JSON-RPC, HTTP-фасад |
| `tests/` | Интеграционные тесты package |

## Типичная цепочка (comet)

```
HTTP → http_domain → longpolling-service → longpolling_domain → pubsub_domain
```
