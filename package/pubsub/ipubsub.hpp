//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once
#include <wfc/iinterface.hpp>
#include <pubsub/api/publish.hpp>
#include <pubsub/api/subscribe.hpp>
#include <pubsub/api/describe.hpp>
#include <pubsub/api/get_messages.hpp>

namespace wfc{ namespace pubsub{

struct ipubsub: iinterface
{
  virtual ~ipubsub(){}
  virtual void publish( request::publish::ptr req, response::publish::callback cb ) = 0;
  virtual void subscribe( request::subscribe::ptr req, response::subscribe::callback cb,
                          io_id_t io_id, std::weak_ptr<ipubsub> ) = 0;
  virtual void describe( request::describe::ptr req, response::describe::callback cb, io_id_t io_id) = 0;
  virtual void get_messages( request::get_messages::ptr req, response::get_messages::callback cb ) = 0;
};

}}

