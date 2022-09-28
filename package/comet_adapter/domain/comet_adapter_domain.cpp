//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "comet_adapter_domain.hpp"
#include <pubsub/logger.hpp>
#include <wfc/logger.hpp>
#include <wfc/memory.hpp>
#include <iostream>

namespace wfc{ namespace comet_adapter{

namespace {
  class adapter_impl
    : public pubsub::ipubsub
  {
  public:
    adapter_impl(std::weak_ptr<icomet_adapter> ca )
      : _ca(ca)
    {}

    virtual void publish( pubsub::request::publish::ptr req, pubsub::response::publish::callback cb ) override
    {
      if ( auto pca = _ca.lock() )
      {
        for ( auto& m : req->messages )
        {
          auto req2 = std::make_unique<request::publish>();
          req2->channel = std::move(m.channel);
          static_cast<pubsub::message&>(*req2) = std::move( static_cast<pubsub::message&>(m) );
          pca->publish(std::move(req2), nullptr );
        }
      }
      if ( cb != nullptr )
      {
        cb( std::make_unique<pubsub::response::publish>() );
      }
    }

    virtual void subscribe( pubsub::request::subscribe::ptr, pubsub::response::subscribe::callback,
                          io_id_t, std::weak_ptr<ipubsub> ) override
    {
      abort();
    }

    virtual void describe( pubsub::request::describe::ptr, pubsub::response::describe::callback, io_id_t) override
    {
      abort();
    }

    virtual void get_messages( pubsub::request::get_messages::ptr, pubsub::response::get_messages::callback) override
    {
      abort();
    }

  private:
    std::weak_ptr<icomet_adapter> _ca;
  };
}

comet_adapter_domain::comet_adapter_domain()
{
}

void comet_adapter_domain::configure()
{
}

void comet_adapter_domain::initialize()
{
  _target = this->get_target<pubsub::ipubsub>( this->options().target );
}

void comet_adapter_domain::reg_io(io_id_t , std::weak_ptr<iinterface>)
{
}

void comet_adapter_domain::unreg_io(io_id_t io_id)
{
  if ( auto pb = _target.lock() )
  {
    pb->unreg_io(io_id);
    std::lock_guard<mutex_type> lk(_mutex);
    _adapter_map.erase(io_id);
  }
}

void comet_adapter_domain::publish( request::publish::ptr req, response::publish::callback cb )
{
  if ( this->bad_request(req, cb) )
    return;

  if ( auto pb = _target.lock() )
  {
    auto req2 = std::make_unique< pubsub::request::publish >();
    pubsub::request::publish::topic tp;
    tp.channel = std::move(req->channel);
    static_cast<pubsub::message&>(tp) = std::move( *req );
    req2->messages.push_back( std::move(tp) );
    if ( cb == nullptr )
    {
      pb->publish( std::move(req2), nullptr );
    }
    else
    {
      pb->publish( std::move(req2), [cb](pubsub::response::publish::ptr) noexcept
      {
        auto res2 = std::make_unique<response::publish::type>();
        *res2 = status::ready;
        cb(std::move(res2));
      });
    }
  }
  else if ( cb != nullptr )
    cb(nullptr);
}

void comet_adapter_domain::mpublish( request::mpublish::ptr req, response::mpublish::callback cb )
{
  if ( this->bad_request(req, cb) )
    return;

  for ( request::publish& msg : *req )
  {
    this->publish(std::make_unique<request::publish>( std::move(msg) ), nullptr );
  }

  if ( cb!=nullptr)
  {
    auto res2 = std::make_unique<response::publish::type>();
    *res2 = status::ready;
    cb(std::move(res2));
  }
}

void comet_adapter_domain::describe( request::describe::ptr req, response::describe::callback cb, io_id_t io_id )
{
  if ( this->bad_request(req, cb) )
    return;

  std::lock_guard<mutex_type> lk(_mutex);
  if ( auto pb = _target.lock() )
  {
    auto req2 = std::make_unique<pubsub::request::describe>();
    req2->channels.push_back(*req);
    pb->describe(std::move(req2), [cb]( pubsub::response::describe::ptr) noexcept
    {
      auto res2 = std::make_unique<response::describe::type>();
      *res2 = status::ready;
      if ( cb!=nullptr )
        cb(std::move(res2));
    }, io_id);
  }
  else if ( cb != nullptr )
    cb(nullptr);
}

void comet_adapter_domain::subscribe( request::subscribe::ptr req, response::subscribe::callback cb,
                              io_id_t io_id, std::weak_ptr<icomet_adapter> wcomet_adapter)
{
  if ( this->bad_request(req, cb) )
    return;


  std::lock_guard<mutex_type> lk(_mutex);

  std::shared_ptr<pubsub::ipubsub> padapt;
  auto itr = _adapter_map.find(io_id);
  if ( itr == _adapter_map.end() )
  {
    padapt = std::make_shared<adapter_impl>(wcomet_adapter);
    _adapter_map[io_id] = padapt;
  }
  else
  {
    padapt = itr->second;
  }

  if ( auto pb = _target.lock() )
  {
    auto req2 = std::make_unique<pubsub::request::subscribe>();
    req2->channels.resize(1);
    req2->channels.back().channel = *req;
    pb->subscribe(std::move(req2), [cb]( pubsub::response::subscribe::ptr) noexcept
    {
      auto res2 = std::make_unique<response::subscribe::type>();
      *res2 = status::ready;
      if ( cb!=nullptr )
        cb(std::move(res2));
    }, io_id, padapt);
  }
  else if ( cb != nullptr )
    cb(nullptr);
}

void comet_adapter_domain::load( request::load::ptr req, response::load::callback cb )
{
  if ( this->notify_ban(req, cb) )
    return;

  if ( auto pb = _target.lock() )
  {
    auto req2 = std::make_unique<pubsub::request::get_messages>();
    req2->channels.resize(1);
    req2->channels.back().channel = req->channel;
    req2->channels.back().limit = req->limit;
    req2->channels.back().cursor = req->cursor;

    pb->get_messages(std::move(req2), [cb]( pubsub::response::get_messages::ptr res) noexcept
    {
      auto res2 = std::make_unique<response::load>();
      if ( res!=nullptr && !res->messages.empty() )
      {
        res2->channel = res->messages[0].channel;
        for ( auto& m : res->messages )
        {
          res2->messages.push_back( static_cast<pubsub::message&&>(m) );
        }
      }
      if ( cb!=nullptr )
        cb(std::move(res2));
    });
  }
  else if ( cb != nullptr )
    cb(nullptr);
}

void comet_adapter_domain::mload( request::mload::ptr req, response::mload::callback cb )
{
  if ( this->notify_ban(req, cb) )
    return;

  if ( auto pb = _target.lock() )
  {
    auto req2 = std::make_unique<pubsub::request::get_messages>();
    auto chlist = std::make_shared<std::set<std::string>>();
    for ( auto& item : *req )
    {
      pubsub::subscribe_params param;
      chlist->insert(item.channel);
      param.channel = std::move(item.channel);
      param.limit = item.limit;
      param.cursor = item.cursor;
      req2->channels.push_back( std::move(param) );
    }
    pb->get_messages(std::move(req2), [cb, chlist]( pubsub::response::get_messages::ptr res) noexcept
    {
      auto res2 = std::make_unique<response::mload::load_list_t>();

      std::map<std::string, response::load::message_list > channel_map;
      if ( res!=nullptr && !res->messages.empty() )
      {
        for ( auto& m: res->messages )
        {
          chlist->erase(m.channel);
          channel_map[m.channel].push_back(static_cast<pubsub::message&&>(m));
        }
      }

      for ( auto& item: channel_map )
      {
        response::load load_channel;
        load_channel.channel = item.first;
        for ( auto& m : item.second)
        {
          load_channel.messages.push_back( static_cast<pubsub::message&&>(m) );
        }
        res2->push_back( std::move(load_channel) );
      }

      for ( const auto& empty_channel : *chlist )
      {
        response::load load_channel;
        load_channel.channel = empty_channel;
        res2->push_back( std::move(load_channel) );
      }


      if ( cb!=nullptr )
        cb(std::move(res2));
    });
  }
  else if ( cb != nullptr )
    cb(nullptr);
}



}}
