
//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "pubsub_domain.hpp"
#include <wfc/logger.hpp>
#include <wfc/memory.hpp>
#include <iostream>
#include <pubsub/logger.hpp>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.


namespace wfc{ namespace pubsub{
  
pubsub_domain::pubsub_domain()
{
}

void pubsub_domain::configure()
{
  pubsub_config opt = this->options();
  _debug_reset = opt.debug_reset;
  opt.ini = this->global()->find_config(opt.ini);
  if ( !opt.rocksdb_disabled && opt.ini.empty())
  {
    if ( opt.ini.empty() )
    { SYSTEM_LOG_FATAL("Не задан ini файл для rocksdb") }
    else
    { SYSTEM_LOG_FATAL("Не найден ini файл для rocksdb:'" << opt.ini << "'") }
    return;
  }
  _pubsub = std::make_shared<pubsub_mt>(opt, opt);
}

void pubsub_domain::reconfigure()
{
  _debug_reset = this->options().debug_reset;
  if ( _pubsub!=nullptr )
    _pubsub->reconfigure(this->options(), this->options());
}


void pubsub_domain::start()
{
  std::weak_ptr<pubsub_domain> wthis = this->shared_from_this();
  this->idle(std::chrono::seconds(this->options().control_s), [wthis]() noexcept
  {
    if ( auto pthis = wthis.lock() )
    {
      size_t messages = 0, channels = 0, subscribers = 0;
      subscribers = pthis->_pubsub->subscriber_count();
      channels = pthis->_pubsub->size(&messages);
      pthis->_pubsub->write_log();
      PUBSUB_LOG_MESSAGE("Subscribers: " << subscribers << " Channels: " << channels << " Messages: " << messages)
      size_t empty_channel = pthis->_pubsub->empty_count();
      size_t removed_messages = pthis->_pubsub->removed_count(true);
      PUBSUB_LOG_MESSAGE("Removed messages: " << removed_messages << " Empty channels: " << empty_channel )

      if ( pthis->_debug_reset )
      {
        pthis->_debug_reset = false;
        PUBSUB_LOG_BEGIN("==== DEBUG RESET ====")
        pthis->_pubsub->reset();
        PUBSUB_LOG_END("==== DEBUG RESET ====")
      }
      return true;
    }
    return false;
  });
}

void pubsub_domain::stop()
{
  if ( _pubsub != nullptr )
    _pubsub->close();
}


void pubsub_domain::publish( request::publish::ptr req, response::publish::callback cb )
{
  if ( this->bad_request(req, cb) )
    return;

  auto res = this->create_response(cb);

  if ( auto ps = _pubsub )
  {
    for (topic& msg : req->messages)
      ps->publish(msg.channel, std::move(static_cast<message&>(msg)) );
  }
  this->send_response( std::move(res), std::move(cb) );
}

void pubsub_domain::get_messages(request::get_messages::ptr req, response::get_messages::callback cb )
{
  if ( this->notify_ban(req, cb) )
    return;

  auto res = this->create_response(cb);

  if ( auto ps = _pubsub )
  {
    for ( subscribe_params params : req->channels )
    {
      pubsub_mt::message_list_t ml;
      ps->get_messages(&ml, params);

      for ( auto& m : ml )
      {
        topic tpk;
        tpk.channel = params.channel; // move(?)
        static_cast<message&>(tpk) = std::move(m);
        res->messages.emplace_back( std::move(tpk) );
      }
    }
  }
  cb( std::move(res) );
}


void pubsub_domain::unreg_io(io_id_t io_id)
{
  if (_pubsub!=nullptr)
    _pubsub->describe(io_id);
  super::unreg_io(io_id);

  std::lock_guard<mutex_type> lk(_mutex);
  _ping_map.erase(io_id);
}

void pubsub_domain::subscribe( request::subscribe::ptr req, response::subscribe::callback cb,
                              io_id_t io_id, std::weak_ptr<isubscriber> wsubscriber)
{
  if ( this->bad_request(req, cb) )
    return;

  auto res = this->create_response(cb);

  for ( subscribe_params params : req->channels )
  {
    pubsub_mt::message_list_t ml;
    params.handler = [wsubscriber](const std::string& channel, const message& msg)
    {
      if ( auto psubscriber = wsubscriber.lock() )
      {
        auto pub_req=std::make_unique<request::publish>();
        topic tpk;
        tpk.channel = channel;
        static_cast<message&>(tpk) = message::copy(msg);
        pub_req->messages.emplace_back( std::move(tpk) );
        psubscriber->notify( std::move(pub_req), nullptr );
      }
    };

    if ( auto ps = _pubsub )
    {
      ps->subscribe(&ml, io_id, params);
      if ( res!=nullptr )
      {
        for ( auto& m : ml )
        {
          topic tpk;
          tpk.channel = params.channel; // move(?)
          static_cast<message&>(tpk) = std::move(m);
          res->messages.emplace_back( std::move(tpk) );
        }
      }
    }
  }

  if ( res != nullptr )
    cb( std::move(res) );

}

void pubsub_domain::describe( request::describe::ptr req, response::describe::callback cb, io_id_t io_id)
{
  if ( this->bad_request(req, cb) )
    return;

  auto res = this->create_response(cb);

  if ( auto ps = _pubsub )
  {
    for (const std::string& channel: req->channels )
    {
      ps->describe(io_id, channel);
    }
  }

  if ( res != nullptr )
    cb( std::move(res) );
}

void pubsub_domain::ping( request::ping::ptr req, response::ping::callback cb, io_id_t io_id)
{
  if ( this->notify_ban(req, cb) )
    return;

  auto res = this->create_response(cb);
  {
    std::lock_guard<mutex_type> lk(_mutex);
    auto itr = _ping_map.find(io_id);
    if ( itr == _ping_map.end() )
    {
      std::stringstream ss;
      ss << boost::uuids::random_generator()();
      itr = _ping_map.insert( std::make_pair(io_id, ss.str() ) ).first;
    }
    res->uuid = itr->second;
  }

  cb( std::move(res) );
}



}}
