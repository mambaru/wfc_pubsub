#include "agent_map.hpp"
#include "logger.hpp"

namespace wfc{ namespace pubsub{

agent_map::~agent_map()
{

}

agent_map::agent_map(const agent_map_options& opt)
  : _opt(opt)
{

}

bool agent_map::create(const agent_options& opt)
{
  if ( agent_ptr ap = this->find_(opt.uuid) )
  {
    return ap->update(opt);
  }

  auto pa = std::make_shared<agent>(opt);
  _agent_map.insert( std::make_pair(opt.uuid, pa) );
  return true;
}


bool agent_map::open(const agent_params& params, const channel_list_t& cl)
{
  if ( agent_ptr ap = this->find_(params.uuid) )
  {
    ap->open(cl);
    for ( const channel_params& cp : cl)
    {
      auto itr = _uc_agent_map.find( std::make_pair(params.uuid, cp.channel) );
      if ( itr == _uc_agent_map.end() )
      {
        _uc_agent_map.insert( std::make_pair( std::make_pair(params.uuid, cp.channel), ap) );
        _cu_agent_map.insert( std::make_pair( std::make_pair(cp.channel, params.uuid), ap) );
      }
    }
    return true;
  }
  return false;
}

bool agent_map::close(const std::string& uuid, const std::vector<std::string>& cl)
{
  if ( agent_ptr ap = this->find_(uuid) )
  {
    ap->close(cl);
    for ( const std::string& channel : cl)
    {
      _uc_agent_map.erase( std::make_pair(uuid, channel));
      _cu_agent_map.erase( std::make_pair(channel, uuid));
    }
    return true;
  }
  return false;
}

size_t agent_map::push(const std::string& channel, const message& m)
{
  size_t count = 0;
  auto itr = _cu_agent_map.lower_bound( std::make_pair(channel, "") );
  for ( ;itr!=_cu_agent_map.end() && itr->first.first == channel; ++itr)
  {
    if ( itr->second->push(channel, message::copy(m) ) )
      ++count;
  }
  return count;
}

bool agent_map::push_to(const std::string& uuid, const std::string& channel, const message& m)
{
  auto itr = _agent_map.find(uuid);
  if ( itr == _agent_map.end() )
    return false;
  return itr->second->push(channel, m);
}


bool agent_map::longpoll(const agent_params& params, const longpoll_hundler_t& handler, longpoll_hundler_t* displaced)
{
  if ( agent_ptr ap = this->find_(params.uuid) )
  {
    if ( params.confirm )
      ap->confirm();
    longpoll_hundler_t prev = ap->longpoll(handler);
    if ( displaced != nullptr && prev )
      *displaced = std::move(prev);
    else if ( prev )
      prev(topic_list_t{});
    return true;
  }
  return false;
}

bool agent_map::get_oid(const std::string& uuid, user_id_t* oid) const
{
  if ( oid == nullptr )
    return false;
  if ( agent_ptr ap = this->find_(uuid) )
  {
    *oid = ap->options().oid;
    return true;
  }
  return false;
}

size_t agent_map::count_without_oid() const
{
  size_t count = 0;
  for ( const auto& pa : _agent_map )
  {
    if ( pa.second != nullptr && pa.second->options().oid == 0 )
      ++count;
  }
  return count;
}

void agent_map::fire(fire_stat *stat, std::vector<std::string>* channels, std::vector<pending_longpoll>* pending_out)
{
  std::deque<std::string> death;
  for ( const auto& pa : _agent_map)
  {
    if ( pa.second!=nullptr )
    {
      pending_longpoll pending;
      pending_longpoll* pout = pending_out != nullptr ? &pending : nullptr;
      if ( !pa.second->fire(stat, pout) )
      {
        death.push_back(pa.first);
      }
      else if ( pending_out != nullptr && pending.handler )
      {
        pending_out->push_back(std::move(pending));
      }
    }
  }
  for (const std::string& uuid : death)
    this->erase_(uuid, channels);

  if ( stat != nullptr )
  {
    stat->active_agents += _agent_map.size();
    stat->active_agents_uc += _cu_agent_map.size();
    stat->deleted_agents += death.size();
  }

  if ( _cu_agent_map.size() != _uc_agent_map.size() )
  {
    LONGPOLL_LOG_ERROR("_cu_agent_map.size() != _uc_agent_map.size() " << _cu_agent_map.size() << "!=" << _uc_agent_map.size() )
  }
}

agent_map::agent_ptr agent_map::find_(const std::string& uuid) const
{
  auto itr = _agent_map.find(uuid);
  if ( itr == _agent_map.end() )
    return nullptr;
  return itr->second;
}

void agent_map::erase_(const std::string& uuid, std::vector<std::string>* channels)
{
  auto itr = _uc_agent_map.lower_bound( std::make_pair(uuid, "") );
  for (;itr!=_uc_agent_map.end() && itr->first.first==uuid;)
  {
    if (channels!=nullptr)
      channels->push_back(itr->first.second);
    _cu_agent_map.erase(std::make_pair( itr->first.second, uuid) );
    _uc_agent_map.erase(itr++);
  }
  _agent_map.erase(uuid);
}

void agent_map::clear()
{
  for ( auto& [k, ap] : _agent_map)
    ap->clear(nullptr);

  _agent_map.clear();
  _uc_agent_map.clear();
  _cu_agent_map.clear();
}


}}
