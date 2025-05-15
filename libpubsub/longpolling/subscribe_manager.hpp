
#pragma once

#include <longpolling/longpolling_options.hpp>
#include <longpolling/types.hpp>
#include <longpolling/logger.hpp>
#include <mutex>
#include <set>
#include <map>
#include <list>
#include <deque>
#include <vector>
#include <iostream>

namespace wfc{ namespace pubsub{

class wait_list
{
public:

  explicit wait_list( time_t lost_delay_s): _lost_delay_s(lost_delay_s) {}

  void wait(const std::vector<std::string>& cl)
  {
    for (const std::string& channel: cl)
    {
      this->ready(channel);
      time_t ttl = time(nullptr) + _lost_delay_s;
      _channel_map[channel] = ttl;
      _time_set.insert( std::make_pair(ttl, channel) );
    }
  }

  void ready(const std::string& name)
  {
    auto itr = _channel_map.find(name);
    if ( itr == _channel_map.end() )
      return;

    _time_set.erase( std::make_pair(itr->second, itr->first) );
    _channel_map.erase(itr);
  }

  void ready(const std::vector<std::string>& cl)
  {
    for (const std::string& channel: cl)
      this->ready(channel);
  }

  size_t pop_outdated(std::set<std::string>* cl)
  {
    size_t count = 0;
    time_t now = time(nullptr);
    for (;; ++count)
    {
      if ( _time_set.empty() )
        return count;

      if ( now < _time_set.begin()->first  )
        return count;

      auto channel = _time_set.begin()->second;
      cl->insert( channel );
      this->ready( channel );
    }
    return count;
  }

  void clear()
  {
    _channel_map.clear();
    _time_set.clear();
  }

  size_t size() const
  {
    if ( _channel_map.size() != _time_set.size() )
    {
      LONGPOLL_LOG_ERROR("Нарушена консистентность wait_list subscribe_manager " << _channel_map.size() << "!=" << _time_set.size() );
      // abort();
    }

    return _time_set.size();
  }
private:
  typedef std::map<std::string, time_t> channel_map_t;
  typedef std::set< std::pair<time_t, std::string> > time_set_t;

  time_t _lost_delay_s = 10;
  channel_map_t _channel_map;
  time_set_t _time_set;
};

class subscribe_manager
{
public:
  virtual ~subscribe_manager();
  explicit subscribe_manager(const longpolling_options& opt);
  subscribe_manager(const subscribe_manager&) = delete;
  subscribe_manager& operator=(const subscribe_manager&) = delete;

  size_t subscribe(const std::string& channel);

  size_t describe(const std::string& channel);

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

  subscribe_stat stat() const;


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

  wait_list _lost_subscribe;
  wait_list _lost_describe;

  bool _describe_suspend = false;
  /*
  // Попыток отписаться от несуществующего канала
  size_t _missed_describe =  0;

  // Попыток отписаться от канала с другими подписчиками
  size_t _ignored_describe =  0;
  */

};

}}


