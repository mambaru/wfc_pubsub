
#include "agent_map_mt.hpp"

namespace wfc{ namespace pubsub{

agent_map_mt::~agent_map_mt()
{

}

agent_map_mt::agent_map_mt(const agent_map_options& opt)
  : _opt(opt)
{
  _agent_map_list.resize(opt.hash_size);
  _mutex_list.resize(opt.hash_size);
  for ( size_t i = 0; i < opt.hash_size; ++i)
  {
    _agent_map_list[i] = std::make_shared<agent_map>(opt);
    _mutex_list[i] = std::make_shared<mutex_type>();
  }
}

bool agent_map_mt::create(const agent_options& opt)
{
  size_t index = index_(opt.uuid);
  std::lock_guard<mutex_type> lk(*_mutex_list[index]);
  return _agent_map_list[index]->create(opt);
}

bool agent_map_mt::open(const agent_params& params, const channel_list_t& cl)
{
  size_t index = index_(params.uuid);
  std::lock_guard<mutex_type> lk(*_mutex_list[index]);
  return _agent_map_list[index]->open(params, cl);
}

bool agent_map_mt::close(const std::string& uuid, const std::vector<std::string>& cl)
{
  size_t index = index_(uuid);
  std::lock_guard<mutex_type> lk(*_mutex_list[index]);
  return _agent_map_list[index]->close(uuid, cl);
}

size_t agent_map_mt::push(const std::string& channel, const message& m)
{
  size_t count = 0;
  for ( size_t index = 0; index < _opt.hash_size; ++index)
  {
    std::lock_guard<mutex_type> lk(*_mutex_list[index]);
    count += _agent_map_list[index]->push(channel, m);
  }
  return count;
}

bool agent_map_mt::push_to(const std::string& uuid, const std::string& channel, const message& m)
{
  size_t index = index_(uuid);
  std::lock_guard<mutex_type> lk(*_mutex_list[index]);
  return _agent_map_list[index]->push_to(uuid, channel, m);
}


bool agent_map_mt::longpoll(const agent_params& params, const longpoll_hundler_t& handler)
{
  size_t index = index_(params.uuid);
  longpoll_hundler_t displaced;
  bool ok = false;
  {
    std::lock_guard<mutex_type> lk(*_mutex_list[index]);
    ok = _agent_map_list[index]->longpoll(params, handler, &displaced);
  }
  // Предыдущий waiter — вне lock, чтобы не дедлочить на re-entry
  if ( displaced )
    displaced(topic_list_t{});
  return ok;
}

bool agent_map_mt::get_oid(const std::string& uuid, user_id_t* oid) const
{
  size_t index = index_(uuid);
  std::lock_guard<mutex_type> lk(*_mutex_list[index]);
  return _agent_map_list[index]->get_oid(uuid, oid);
}

size_t agent_map_mt::count_without_oid() const
{
  size_t count = 0;
  for ( size_t index = 0; index < _opt.hash_size; ++index )
  {
    std::lock_guard<mutex_type> lk(*_mutex_list[index]);
    count += _agent_map_list[index]->count_without_oid();
  }
  return count;
}

void agent_map_mt::fire(fire_stat *stat, std::vector<std::string>* channels)
{
  std::vector<pending_longpoll> pending;
  for ( size_t index = 0; index < _opt.hash_size; ++index)
  {
    std::lock_guard<mutex_type> lk(*_mutex_list[index]);
    _agent_map_list[index]->fire(stat, channels, &pending);
  }
  for ( auto& p : pending )
  {
    if ( p.handler )
      p.handler(p.topics);
  }
}

size_t agent_map_mt::index_( const std::string& uuid ) const
{
  return std::hash<std::string>()(uuid) % _opt.hash_size;
}


void agent_map_mt::clear()
{
  for ( size_t index = 0; index < _opt.hash_size; ++index)
  {
    std::lock_guard<mutex_type> lk(*_mutex_list[index]);
    _agent_map_list[index]->clear();
  }
}

}}
