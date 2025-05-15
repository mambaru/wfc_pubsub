#pragma once
#include <longpolling/agent_options.hpp>
#include <longpolling/channel_params.hpp>
#include <longpolling/types.hpp>
#include <message_queue/channel_map.hpp>

namespace wfc{ namespace pubsub{

/**
 * @brief Накапливает сообщения для конкретного клиента с идентификатором _uuid
 */
class agent
{
public:
  virtual ~agent();
  explicit agent(const agent_options& opt);
  agent(const agent&) = delete;
  agent& operator=(const agent&) = delete;

  bool update(const agent_options& opt);

  // Открываем каналы для накопления сообщений
  // Если канал не открыт, то вызовы this->push для этого канала игнорируются
  void open(const channel_list_t& cl);

  // закрываем каналы для накопления сообщений. Все непереданные сообщения удаляются
  // Но они могут быть подгружены при повторном открытии канала, если указать cursor
  void close( const std::vector<std::string>& channels);

  bool push( const std::string& channel, const message& m);

  // Установить обработчик для longpoll
  void longpoll(const longpoll_hundler_t& handler);

  // Отправить накопленные сообщения. Если не задана опция keep_alive
  // longpoll_hundler обнуляеться до следующего вызова this->longpoll(handler)
  // @params [out] count   количество отправленных сообщений
  // @return false если вышло время жизни
  bool fire(fire_stat *stat);

  // Подверждаем что вызов longpoll_hundler доставил сообщения клиенту
  void confirm();

  const channel_map& map() const;

  static std::string create_uuid();

  void clear(std::vector<std::string>* channels);

private:
  bool find_(const std::string& channel, channel_params* cpp) const;
  void open_(const channel_params& cp);
  bool close_(const std::string& channel);

  bool rewait_();
  void prio_wait_();
private:
  time_t _deathtime = 0;
  time_t _polltime = 0;
  agent_options _opt;
  longpoll_hundler_t _longpoll_hundler;
  channel_map _channel_map;
  channel_list_t _channel_list;
  topic_list_t _wait_list;
  topic_list_t _prio_list;
};

}}

