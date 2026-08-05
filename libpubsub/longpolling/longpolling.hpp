
#pragma once

#include <longpolling/longpolling_options.hpp>
#include <longpolling/agent_map_mt.hpp>
#include <longpolling/subscribe_manager.hpp>
#include <longpolling/fire_stat.hpp>
#include <message_queue/channel_map.hpp>
#include <mutex>
#include <set>
#include <list>

namespace wfc{ namespace pubsub{

class longpolling
{
public:
  virtual ~longpolling();
  explicit longpolling(const longpolling_options& opt);
  longpolling(const longpolling&) = delete;
  longpolling& operator=(const longpolling&) = delete;

  bool create(const agent_options& opt);

  // Открываем каналы для накопления сообщений
  // Если канал не открыт, то вызовы this->push для этого канала игнорируются
  bool open(const agent_params& params, const channel_list_t& cl);

  bool close(const std::vector<std::string>& cl);

  // закрываем каналы для накопления сообщений. Все непереданные сообщения удаляются
  // Но они могут быть подгружены при повторном открытии канала, если указать cursor
  bool close(const std::string& uuid, const std::vector<std::string>& cl);

  size_t push(const std::string& channel, const message& m);

  // Установить обработчик для longpoll
  bool longpoll(const agent_params& params, const longpoll_hundler_t& handler);

  // Отправить накопленные сообщения. Если не задана опция keep_alive
  // longpoll_hundler обнуляеться до следующего вызова this->longpoll(handler)
  void fire(fire_stat *stat);

  bool pop_for_subscribe(std::vector<std::string>* cl, size_t limit);

  bool pop_for_describe(std::vector<std::string>* cl, size_t limit);

  void confirm_for_subscribe( const std::vector<std::string>& cl);

  void confirm_for_describe( const std::vector<std::string>& cl);

  void rollback_for_subscribe( const std::vector<std::string>& cl);

  void rollback_for_describe( const std::vector<std::string>& cl);

  // Серер не доступен, требуется переподписка для всех каналов
  void resubscribe();

  // Подписаться на канал без агента (системные каналы, например авторизация)
  void ensure_subscribed(const std::string& channel);

  void unsubscribe_channel(const std::string& channel);

  bool get_agent_oid(const std::string& uuid, user_id_t* oid) const;

  size_t count_agents_without_oid() const;

  void clear();

  hub_queue_stat get_hub_queue_stat() const;

private:

  typedef std::mutex mutex_type;

  agent_map_mt _agent_map_mt;
  channel_map _common_hub;
  subscribe_manager _subscribe_manager;
  mutable mutex_type _mutex;
};

}}


