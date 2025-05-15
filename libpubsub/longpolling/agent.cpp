
#include "agent.hpp"
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <sstream>

namespace wfc{ namespace pubsub{

agent::~agent()
{
}

agent::agent(const agent_options& opt)
  : _deathtime(time(nullptr) + opt.agent_lifetime)
  , _polltime(time(nullptr) + opt.longpoll_timeout)
  , _opt(opt)

{
}

bool agent::update(const agent_options& opt)
{
  if ( _opt.key != opt.key )
    return false;

  _opt = opt;
  return true;
}


const channel_map& agent::map() const
{
  return _channel_map;
}

void agent::open(const channel_list_t& cl)
{
  for ( const auto& c: cl)
    this->open_(c);
}

void agent::close( const std::vector<std::string>& channels)
{
  for ( const auto& c: channels)
  {
    this->close_(c);
    _channel_map.erase(c);
  }
}

bool agent::push( const std::string& channel, const message& m)
{
  channel_params cp;
  if ( !this->find_(channel, &cp) )
    return false;


  if ( m.cursor!=0 && cp.cursor!=0 && (m.cursor <= cp.cursor) )
    return false;

  message mm = message::copy(m);

  if ( auto pm = _channel_map.push(channel, mm) )
  {
    _prio_list.push_back( topic{std::move(pm->get()),channel} );
    if ( _prio_list.size() > _opt.prio_limit )
    {
      _prio_list.erase(_prio_list.begin());
      //_prio_list.pop_front();
    }
  }
  return true;
}

void agent::longpoll(const longpoll_hundler_t& handler)
{
  _longpoll_hundler = handler;
  _deathtime = time(nullptr) + _opt.agent_lifetime;
}

bool agent::fire(fire_stat *stat)
{
  time_t now = time(nullptr);

  if ( stat!=nullptr)
    stat->remove_death = _channel_map.removed_count(true);

  if ( _deathtime <= now )
    return false;

  if ( _longpoll_hundler==nullptr )
  {
    if ( stat!=nullptr)
    {
      stat->wait_messages += _wait_list.size();
      stat->active_channels += _channel_map.size(&(stat->stored_messages));
    }
    return true;
  }

  // Прогоняем лист ожидания, чтобы удалить устаревшие
  this->rewait_();

  this->prio_wait_();

  if ( _wait_list.size() < _opt.longpoll_limit )
  {
    size_t limit = _opt.longpoll_limit - _wait_list.size();
    _channel_map.takeaway( limit, &_wait_list);
  }

  bool ready = !_wait_list.empty();
  if ( !ready )
  {
    ready  = _polltime <= now;
  }

  if ( ready )
  {
    if ( stat!=nullptr)
      stat->sended_messages += _wait_list.size();
    _polltime = now + _opt.longpoll_timeout;
    _longpoll_hundler( _wait_list );
    _longpoll_hundler=nullptr;
  }
  if ( stat!=nullptr)
    stat->active_channels += _channel_map.size(&(stat->stored_messages));
  return true;
}

void agent::confirm()
{
  _wait_list.clear();
}

std::string agent::create_uuid()
{
  std::stringstream ss;
  ss << boost::uuids::random_generator()();
  return ss.str();
}

bool agent::rewait_()
{
  if ( !_wait_list.empty() )
  {
    // Временная карта каналов
    channel_map cm;
    topic_list_t wl;
    _wait_list.swap(wl);
    // Сначала загоняем текущий лист ожидания
    for ( topic& tpc : wl)
    {
      if ( auto pm = cm.push(tpc.channel, std::move(tpc) ) )
      {
        // Приоритетные сообщения
        _wait_list.push_back(topic{std::move(pm->get()),tpc.channel});
      }
    }

    // Докидываем новые сообщения (чтобы применить actions)
    size_t count = 0;
    cm.size(&count);
    if ( count + _wait_list.size() < _opt.longpoll_limit )
    {
      size_t limit = _opt.longpoll_limit - count - _wait_list.size();
      wl.clear();
      _channel_map.takeaway(limit, &wl);
      for ( topic& tpc : wl)
        cm.push(tpc.channel, std::move(tpc) );
    }

    if ( _wait_list.size() < _opt.longpoll_limit )
      cm.takeaway(_opt.longpoll_limit - _wait_list.size(), &_wait_list);
    return true;
  }
  return false;
}

void agent::prio_wait_()
{
  size_t limit = _opt.longpoll_limit;
  if ( _wait_list.size() < limit )
  {
    limit -= _wait_list.size();

    if ( _prio_list.size() <= limit )
    {
      //limit -= _prio_list.size();
      std::move(_prio_list.begin(), _prio_list.end(), std::back_inserter(_wait_list));
      _prio_list.clear();
    }
    else
    {
      auto beg = _prio_list.begin();
      auto end = beg;
      std::advance(end, limit);
      std::move(beg, end, std::back_inserter(_wait_list));
      _prio_list.erase(beg, end);
      limit = 0;
    }
  }
}

void agent::open_(const channel_params& cp)
{
  auto itr = std::lower_bound(
    _channel_list.begin(),
    _channel_list.end(),
    cp,
    [](const channel_params& left, const channel_params& right) -> bool
    {
      return left.channel < right.channel;
    }
  );
  if ( itr!=_channel_list.end() && itr->channel == cp.channel )
    *itr=cp;
  else
    _channel_list.insert(itr, cp);
}

bool agent::find_(const std::string& channel, channel_params* cpp) const
{
  auto itr = std::lower_bound(
    _channel_list.begin(),
    _channel_list.end(),
    channel_params{channel},
    [](const channel_params& left, const channel_params& right) -> bool
    {
      return left.channel < right.channel;
    }
  );
  if ( itr == _channel_list.end() || itr->channel!=channel)
    return false;
  *cpp = *itr;
  return true;
}

bool agent::close_(const std::string& channel)
{
  auto itr = std::lower_bound(
    _channel_list.begin(),
    _channel_list.end(),
    channel_params{channel},
    [](const channel_params& left, const channel_params& right) -> bool
    {
      return left.channel < right.channel;
    }
  );
  if ( itr == _channel_list.end() || itr->channel!=channel)
    return false;
  _channel_list.erase(itr);
  return true;
}

void agent::clear(std::vector<std::string>* channels)
{
  _channel_list.clear();
  _channel_map.clear(channels);
  _wait_list.clear();
  _prio_list.clear();
}

}}

