#include "subscribe_manager.hpp"
#include "logger.hpp"
#include <unistd.h>

namespace wfc{ namespace pubsub{

subscribe_manager::~subscribe_manager()
{

}

subscribe_manager::subscribe_manager(const longpolling_options& opt)
  : _describe_delay_s(opt.describe_delay_s)
  , _lost_subscribe(opt.lost_delay_s)
  , _lost_describe(opt.lost_delay_s)
  , _describe_suspend(opt.describe_suspend)
{
}

size_t subscribe_manager::subscribe(const std::string& channel)
{
  _wait_describe.erase(channel);
  auto itr = _counter_map.find(channel);
  if ( itr != _counter_map.end() )
  {
    return ++itr->second; // пересчитать при отписке (для фикса дважды открытия)
  }

  _counter_map[channel] = 1;
  _wait_subscribe.insert(channel);
  return 1;
}

size_t subscribe_manager::describe(const std::string& channel)
{
  if ( _describe_suspend )
    return 0;

  auto itr = _counter_map.find(channel);

  if ( itr == _counter_map.end() )
    return 0;

  if ( itr->second > 0)
    if ( --itr->second > 0 )
      return itr->second;

  _describe_delay.push_back( std::make_pair( time(nullptr) + _describe_delay_s, channel ) );
  return 0;
}

bool subscribe_manager::pop_for_subscribe(std::vector<std::string>* cl, size_t limit)
{
  if ( size_t lost = _lost_subscribe.pop_outdated(&_wait_subscribe) )
  {
    // Такого не должно происходиить, поэтому это ошибка, а не предупреждение
    LONGPOLL_LOG_ERROR("Таймауты на subscribe от хаба в количестве: " << lost << ". Ожидается повторный запрос.");
  }
  return this->pop_for_(&_wait_subscribe, &_lost_subscribe, cl, limit);
}

bool subscribe_manager::pop_for_describe(std::vector<std::string>* cl, size_t limit)
{
  if ( size_t lost = _lost_describe.pop_outdated(&_wait_describe) )
  {
    // Такого не должно происходиить, поэтому это ошибка, а не предупреждение
    LONGPOLL_LOG_ERROR("Таймауты на describe от хаба в количестве: " << lost << ". Ожидается повторный запрос.");
  }

  time_t now = time(nullptr);
  while ( !_describe_delay.empty() )
  {
    if ( _describe_delay.front().first <= now )
    {
      std::string channel = _describe_delay.front().second;
      _describe_delay.pop_front();

      if ( _wait_subscribe.count(channel) != 0 )
        continue;

      auto itr = _counter_map.find(channel);
      if ( itr!=_counter_map.end()  )
      {
        if ( itr->second!=0 )
          continue;
        _counter_map.erase(itr);
      }

      _wait_describe.insert(channel);
    }
    else
      break;
  }
  return this->pop_for_(&_wait_describe, &_lost_describe,  cl, limit);
}

void subscribe_manager::confirm_for_subscribe( const std::vector<std::string>& cl)
{
  _lost_subscribe.ready(cl);
}

void subscribe_manager::confirm_for_describe( const std::vector<std::string>& cl)
{
  for (const auto& s : cl)
  {
    auto itr = _counter_map.find(s);
    if ( itr!=_counter_map.end() )
    {
      if ( itr->second==0 )
      {
        if ( _wait_subscribe.count(s) == 0 )
          _counter_map.erase(itr);
      }
    }
  }
  _lost_describe.ready(cl);
}


void subscribe_manager::rollback_for_subscribe( const std::vector<std::string>& cl)
{
  _lost_subscribe.ready(cl);
  this->rollback_for_(&_wait_subscribe, &_wait_describe, cl);
}

void subscribe_manager::rollback_for_describe( const std::vector<std::string>& cl)
{
  _lost_describe.ready(cl);
  this->rollback_for_(&_wait_describe, &_wait_subscribe, cl);
}


void subscribe_manager::resubscribe()
{
  counter_map_t cm;
  _counter_map.swap(cm);
  this->clear();
  for ( const auto& [channel, count] : cm )
  {
    if ( count > 0 )
      _wait_subscribe.insert(channel);
  }
}

void subscribe_manager::clear()
{
  _counter_map.clear();
  _wait_subscribe.clear();
  _wait_describe.clear();
  _lost_subscribe.clear();
  _lost_describe.clear();
}


bool subscribe_manager::pop_for_(channel_set_t* channel_set, wait_list* wl, std::vector<std::string>* cl, size_t limit)
{
  if ( cl == nullptr )
    return false;

  cl->reserve( limit );
  if ( channel_set->size() < limit )
  {
    std::move( channel_set->begin(), channel_set->end(), std::back_inserter(*cl) );
    channel_set->clear();
    wl->wait(*cl);
    return false;
  }

  for ( ;limit!=0; --limit )
  {
    cl->push_back( *channel_set->begin() );
    channel_set->erase(channel_set->begin());
  }
  wl->wait(*cl);
  return true;
}

void subscribe_manager::rollback_for_( channel_set_t* channel_set, channel_set_t* channel_rev, const std::vector<std::string>& cl)
{
  for ( const auto& ch : cl )
  {
    // Если во время неудачного запроса на подписку не произошла отписка
    // или наоборот
    if ( channel_rev->count(ch) == 0  )
    {
      channel_set->insert(ch);
    }
  }
}

bool subscribe_manager::has(const std::string& channel) const
{
  return _counter_map.count(channel) > 0;
}

subscribe_stat subscribe_manager::stat() const
{
  subscribe_stat sstat;

  sstat.counter_map = _counter_map.size();
  sstat.wait_subscribe = _wait_subscribe.size();
  sstat.wait_describe = _wait_describe.size();
  sstat.describe_delay = _describe_delay.size();
  sstat.lost_subscribe = _lost_subscribe.size();
  sstat.lost_describe = _lost_describe.size();

  return sstat;
}


}}
