#pragma once

#include <longpolling/longpolling_options.hpp>
#include <longpolling/hub_queue_stat.hpp>
#include <longpolling/wait_list.hpp>
#include <set>
#include <map>
#include <list>
#include <vector>

namespace wfc{ namespace pubsub{

class subscribe_manager
{
public:
  virtual ~subscribe_manager();
  explicit subscribe_manager(const longpolling_options& opt);
  subscribe_manager(const subscribe_manager&) = delete;
  subscribe_manager& operator=(const subscribe_manager&) = delete;

  size_t subscribe(const std::string& channel);

  size_t describe(const std::string& channel);

  // Немедленная отписка (без describe_delay), для shutdown
  void unsubscribe_now(const std::string& channel);

  bool pop_for_subscribe(std::vector<std::string>* cl, size_t limit);

  bool pop_for_describe(std::vector<std::string>* cl, size_t limit);

  void confirm_for_subscribe( const std::vector<std::string>& cl);

  void confirm_for_describe( const std::vector<std::string>& cl);

  void rollback_for_subscribe( const std::vector<std::string>& cl);

  void rollback_for_describe( const std::vector<std::string>& cl);

  // Серер не доступен, требуется переподписка для всех каналов
  void resubscribe();

  void clear();

  bool has(const std::string&) const;

  hub_queue_stat stat() const;


private:
  typedef std::map<std::string, size_t> counter_map_t;
  typedef std::set<std::string> channel_set_t;
  typedef std::list< std::pair<time_t, std::string> > delay_list_t;

  bool pop_for_(channel_set_t* channel_set, wait_list* wl, std::vector<std::string>* cl, size_t limit);
  void rollback_for_( channel_set_t* channel_set, channel_set_t* channel_rev, const std::vector<std::string>& cl);

private:

  // счетчики подписчиков на канал (сколько агентов висит на канале)
  counter_map_t _counter_map;
  // спискок каналов для подписке на хабе
  channel_set_t _wait_subscribe;
  // спискок каналов для отписки на хабе
  channel_set_t _wait_describe;
  // список каналов на который не подписан ни один агент более _describe_delay_s секунд
  delay_list_t _describe_delay;
  // Сколько держать канал у которого не осталось агентов, предже чем отправить отписку на хаб
  time_t _describe_delay_s = 3600;

  wait_list _inflight_subscribe;
  wait_list _inflight_describe;

  bool _describe_suspend = false;
  /*
  // Попыток отписаться от несуществующего канала
  size_t _missed_describe =  0;

  // Попыток отписаться от канала с другими подписчиками
  size_t _ignored_describe =  0;
  */

};

}}
