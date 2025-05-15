#include "longpolling.hpp"
#include "logger.hpp"
namespace wfc{ namespace pubsub{

longpolling::~longpolling()
{

}

longpolling::longpolling(const longpolling_options& opt)
  : _agent_map_mt(opt)
  , _subscribe_manager(opt)
{
}

bool longpolling::create(const agent_options& opt)
{
  return _agent_map_mt.create(opt);
}

bool longpolling::open(const agent_params& params, const channel_list_t& cl)
{
  if ( !_agent_map_mt.open(params, cl) )
    return false;

  std::lock_guard<mutex_type> lk(_mutex);

  for ( const channel_params& cp : cl)
  {
    _subscribe_manager.subscribe(cp.channel);

    channel_map::message_list_t ml;
    _common_hub.get_messages(&ml, cp.channel, cp.cursor, cp.max_size);
    for ( message& m : ml )
      _agent_map_mt.push_to(params.uuid, cp.channel, m);
  }
  return true;
}

bool longpolling::close(const std::string& uuid, const std::vector<std::string>& cl)
{
   if ( !_agent_map_mt.close(uuid, cl) )
    return false;

  std::lock_guard<mutex_type> lk(_mutex);

  for ( const std::string& cp : cl)
  {
    _subscribe_manager.describe(cp);
  }
  return true;
}


size_t longpolling::push(const std::string& channel, const message& m)
{
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( !_subscribe_manager.has(channel) )
    {
      LONGPOLL_LOG_WARNING("Сообщение для несуществующего канала: " << channel )
    }
    _common_hub.push(channel, m);
  }
  return _agent_map_mt.push(channel, m);
}

bool longpolling::longpoll(const agent_params& params, const longpoll_hundler_t& handler)
{
  return _agent_map_mt.longpoll(params, handler);
}

void longpolling::fire(fire_stat *stat)
{
  std::vector<std::string> closed_channels;
  _agent_map_mt.fire(stat, &closed_channels);
  {
    std::lock_guard<mutex_type> lk(_mutex);
    for (const auto& ch: closed_channels)
    {
      ++stat->dead_describe;
      _subscribe_manager.describe(ch);
    }
    stat->hub_active_channels += _common_hub.size( &(stat->hub_stored_messages) );
    stat->hub_remove_death += _common_hub.remove_death();
    stat->subs_stat += _subscribe_manager.stat();
  }
}

bool longpolling::pop_for_subscribe(std::vector<std::string>* cl, size_t limit)
{
  std::lock_guard<mutex_type> lk(_mutex);
  return _subscribe_manager.pop_for_subscribe(cl, limit);
}

bool longpolling::pop_for_describe(std::vector<std::string>* cl, size_t limit)
{
  std::lock_guard<mutex_type> lk(_mutex);
  return _subscribe_manager.pop_for_describe(cl, limit);
}

void longpolling::confirm_for_subscribe( const std::vector<std::string>& cl)
{
  std::lock_guard<mutex_type> lk(_mutex);
  return _subscribe_manager.confirm_for_subscribe(cl);
}

void longpolling::confirm_for_describe( const std::vector<std::string>& cl)
{
  std::lock_guard<mutex_type> lk(_mutex);
  return _subscribe_manager.confirm_for_describe(cl);
}

void longpolling::rollback_for_subscribe( const std::vector<std::string>& cl)
{
  std::lock_guard<mutex_type> lk(_mutex);
  _subscribe_manager.rollback_for_subscribe(cl);
}

void longpolling::rollback_for_describe( const std::vector<std::string>& cl)
{
  std::lock_guard<mutex_type> lk(_mutex);
  _subscribe_manager.rollback_for_describe(cl);
}

void longpolling::resubscribe()
{
  std::lock_guard<mutex_type> lk(_mutex);
  _subscribe_manager.resubscribe();
}

void longpolling::clear()
{
  _agent_map_mt.clear();
  std::lock_guard<mutex_type> lk(_mutex);
  _common_hub.clear(nullptr);
  _subscribe_manager.clear();
}

}}


