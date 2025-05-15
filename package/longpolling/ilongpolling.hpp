//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once
#include <wfc/iinterface.hpp>
#include <longpolling/api/create.hpp>
#include <longpolling/api/open.hpp>
#include <longpolling/api/close.hpp>
#include <longpolling/api/longpoll.hpp>
#include <longpolling/api/publish.hpp>
#include <longpolling/api/subscribe.hpp>
#include <longpolling/icomet_subscriber.hpp>
#include <pubsub/ipubsub.hpp>

namespace wfc{ namespace pubsub{

struct ilongpolling: isubscriber
{
  virtual ~ilongpolling(){}
  virtual void create1( request::create::ptr req, response::create::callback cb ) = 0;
  virtual void open( request::open::ptr req, response::open::callback cb ) = 0;
  virtual void close( request::close::ptr req, response::close::callback cb ) = 0;
  virtual void longpoll( request::longpoll::ptr req, response::longpoll::callback cb ) = 0;
  virtual void publish( request::publish::ptr req, response::publish::callback cb ) = 0;

  virtual void subscribe(
    request::longpoll::ptr req, depr::response::subscribe::callback cb,
    io_id_t io_id, std::weak_ptr<depr::icomet_subscriber>) = 0;

  virtual void create_depr( depr::request::create::ptr req, depr::response::create::callback cb) = 0;

};
}}


