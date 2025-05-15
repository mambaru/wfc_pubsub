#pragma once

#include <longpolling/agent_params.hpp>
#include <longpolling/agent.hpp>
#include <longpolling/agent_map_options.hpp>
#include <longpolling/types.hpp>

namespace wfc{ namespace pubsub{

/**
 * @brief Накапливает сообщения для конкретного клиента с идентификатором _uuid
 */
class agent_map
{
public:
  virtual ~agent_map();
  explicit agent_map(const agent_map_options& opt);
  agent_map(const agent_map&) = delete;
  agent_map& operator=(const agent_map&) = delete;

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
  typedef std::shared_ptr<agent> agent_ptr;
  typedef std::map<std::string, agent_ptr> agent_map_t;
  typedef std::pair<std::string, std::string> uuid_channel_t;
  typedef std::pair<std::string, std::string> channel_uud_t;
  typedef std::map<uuid_channel_t, agent_ptr> uc_agent_map_t;
  typedef std::map<channel_uud_t, agent_ptr> cu_agent_map_t;

  agent_ptr find_(const std::string& uuid) const;
  void erase_(const std::string& uuid, std::vector<std::string>* channels);
private:

  agent_map_options _opt;
  agent_map_t _agent_map;
  uc_agent_map_t _uc_agent_map;
  cu_agent_map_t _cu_agent_map;
};

}}

