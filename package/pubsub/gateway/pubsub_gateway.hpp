//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <pubsub/ipubsub.hpp>
#include <pubsub/api/ping_json.hpp>
#include <pubsub/api/publish_json.hpp>
#include <pubsub/api/subscribe_json.hpp>
#include <pubsub/api/describe_json.hpp>
#include <pubsub/api/get_messages_json.hpp>
#include <wfc/jsonrpc.hpp>


namespace wfc{ namespace pubsub{

JSONRPC_TAG(ping)
JSONRPC_TAG(publish)
JSONRPC_TAG(subscribe)
JSONRPC_TAG(describe)
JSONRPC_TAG(get_messages)

JSONRPC_TAG2(_subscriber_publish_, "publish")

struct pubsub_gateway_method_list: public jsonrpc::method_list
<
  jsonrpc::target<isubscriber>,
  jsonrpc::dual_method< _publish_, request::publish_json, response::publish_json, isubscriber, &isubscriber::notify>,

  jsonrpc::interface_<ipubsub>,
  jsonrpc::call_method< _subscribe_, request::subscribe_json, response::subscribe_json>,
  jsonrpc::call_method< _describe_, request::describe_json, response::describe_json>,
  jsonrpc::call_method< _get_messages_, request::get_messages_json, response::get_messages_json>,
  jsonrpc::call_method< _ping_, request::ping_json, response::ping_json>

>
{};

template<typename Base>
class pubsub_interface
  : public Base
{
public:
  
  virtual void publish(request::publish::ptr req, response::publish::callback cb ) override
  {
    this->template call< _publish_ >( std::move(req), cb, nullptr);
  }

  virtual void subscribe(request::subscribe::ptr req, response::subscribe::callback cb, iinterface::io_id_t, std::weak_ptr<isubscriber> ) override
  {
    this->template call< _subscribe_ >( std::move(req), cb, nullptr);
  }

  virtual void describe(request::describe::ptr req, response::describe::callback cb, iinterface::io_id_t ) override
  {
    this->template call< _describe_ >( std::move(req), cb, nullptr);
  }

  virtual void get_messages(request::get_messages::ptr req, response::get_messages::callback cb ) override
  {
    this->template call< _get_messages_ >( std::move(req), cb, nullptr);
  }

  virtual void ping(request::ping::ptr req, response::ping::callback cb, iinterface::io_id_t ) override
  {
    this->template call< _ping_ >( std::move(req), cb, nullptr);
  }

};

}}
