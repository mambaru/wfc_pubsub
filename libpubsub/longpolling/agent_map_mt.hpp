#pragma once

#include <longpolling/agent_map.hpp>
#include <mutex>

namespace wfc{ namespace pubsub{

/**
 * @brief Накапливает сообщения для конкретного клиента с идентификатором _uuid
 */
class agent_map_mt
{
public:
  virtual ~agent_map_mt();
  explicit agent_map_mt(const agent_map_options& opt);
  agent_map_mt(const agent_map_mt&) = delete;
  agent_map_mt& operator=(const agent_map_mt&) = delete;

  bool create(const agent_options& opt);

  // Открываем каналы для накопления сообщений
  // Если канал не открыт, то вызовы this->push для этого канала игнорируются
  bool open(const agent_params& params, const channel_list_t& cl);

  // закрываем каналы для накопления сообщений. Все непереданные сообщения удаляются
  // Но они могут быть подгружены при повторном открытии канала, если указать cursor
  bool close(const std::string& uuid, const std::vector<std::string>& cl);

  size_t push(const std::string& channel, const message& m);

  bool push_to(const std::string& uuid, const std::string& channel, const message& m);

  // Установить обработчик для longpoll
  bool longpoll(const agent_params& params, const longpoll_hundler_t& handler);

  // Отправить накопленные сообщения. Если не задана опция keep_alive
  // longpoll_hundler обнуляеться до следующего вызова this->longpoll(handler)
  void fire(fire_stat *stat, std::vector<std::string>* channels);

  void clear();
private:
  size_t index_( const std::string& uuid ) const;

private:
  typedef std::shared_ptr<agent_map> agent_map_ptr;
  typedef std::vector<agent_map_ptr> agent_map_list_t;
  typedef std::mutex mutex_type;
  typedef std::shared_ptr<mutex_type> mutex_ptr;
  typedef std::vector<mutex_ptr> mutex_list_t;
  agent_map_options _opt;
  agent_map_list_t _agent_map_list;
  mutex_list_t _mutex_list;
};

}}


