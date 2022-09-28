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

namespace wfc{ namespace pubsub{
  
pubsub_domain::pubsub_domain()
{
}

void pubsub_domain::configure()
{
  _debug_reset = this->options().debug_reset;
  _pubsub = std::make_shared<pubsub_mt>( this->options(), this->options());
}

void pubsub_domain::reconfigure()
{
  _debug_reset = this->options().debug_reset;
  _pubsub->reconfigure(this->options(), this->options());
}


void pubsub_domain::start()
{
  std::weak_ptr<pubsub_domain> wthis = this->shared_from_this();
  this->idle(std::chrono::seconds(10), [wthis]() noexcept
  {
    if ( auto pthis = wthis.lock() )
    {
      size_t messages = 0, channels = 0, subscribers = 0;
      subscribers = pthis->_pubsub->subscriber_count();
      channels = pthis->_pubsub->size(&messages);
      pthis->_pubsub->write_log();
      PUBSUB_LOG_MESSAGE("Subscribers: " << subscribers << " Channels: " << channels << " Messages: " << messages)
      size_t empty_channel = pthis->_pubsub->empty_count();
      size_t removed_messages = pthis->_pubsub->get_removed(true);
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
  _pubsub->close();
}


void pubsub_domain::publish( request::publish::ptr req, response::publish::callback cb )
{
  if ( this->bad_request(req, cb) )
    return;

  auto res = this->create_response(cb);

  for (request::publish::topic& msg : req->messages)
    _pubsub->publish(msg.channel, std::move(static_cast<message&>(msg)) );

  this->send_response( std::move(res), std::move(cb) );
}

void pubsub_domain::get_messages(request::get_messages::ptr req, response::get_messages::callback cb )
{
  if ( this->notify_ban(req, cb) )
    return;

  auto res = this->create_response(cb);

  for ( subscribe_params params : req->channels )
  {
    pubsub_mt::message_list_t ml;
    _pubsub->get_messages(&ml, params);

    for ( auto& m : ml )
    {
      request::publish::topic tpk;
      tpk.channel = params.channel; // move(?)
      static_cast<message&>(tpk) = std::move(m);
      res->messages.emplace_back( std::move(tpk) );
    }
  }
  cb( std::move(res) );
}


void pubsub_domain::unreg_io(io_id_t io_id)
{
  _pubsub->describe(io_id);
}

void pubsub_domain::subscribe( request::subscribe::ptr req, response::subscribe::callback cb,
                              io_id_t io_id, std::weak_ptr<ipubsub> wpubsub)
{
  if ( this->bad_request(req, cb) )
    return;

  auto res = this->create_response(cb);

  for ( subscribe_params params : req->channels )
  {
    pubsub_mt::message_list_t ml;
    params.handler = [wpubsub](const std::string& channel, const message& msg)
    {
      if ( auto ppubsub = wpubsub.lock() )
      {
        auto pub_req=std::make_unique<request::publish>();
        request::publish::topic tpk;
        tpk.channel = channel;
        static_cast<message&>(tpk) = stored_message::copy(msg);
        pub_req->messages.emplace_back( std::move(tpk) );
        ppubsub->publish( std::move(pub_req), nullptr );
      }
    };

    _pubsub->subscribe(&ml, io_id, params);
    if ( res!=nullptr )
    {
      for ( auto& m : ml )
      {
        request::publish::topic tpk;
        tpk.channel = params.channel; // move(?)
        static_cast<message&>(tpk) = std::move(m);
        res->messages.emplace_back( std::move(tpk) );
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

  for (const std::string& channel: req->channels )
  {
    _pubsub->describe(io_id, channel);
  }

  if ( res != nullptr )
    cb( std::move(res) );
}
}}
