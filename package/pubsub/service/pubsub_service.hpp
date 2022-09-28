#pragma once

#include <pubsub/ipubsub.hpp>
#include <pubsub/api/publish_json.hpp>
#include <pubsub/api/subscribe_json.hpp>
#include <pubsub/api/describe_json.hpp>
#include <pubsub/api/get_messages_json.hpp>
#include <wfc/jsonrpc.hpp>

namespace wfc{ namespace pubsub{

JSONRPC_TAG(publish)
JSONRPC_TAG(subscribe)
JSONRPC_TAG(describe)
JSONRPC_TAG(get_messages)

template<typename Base>
class pubsub_interface: public Base
{
public:
  typedef Base super;
  typedef typename super::io_id_t io_id_t;

  virtual void publish(request::publish::ptr req, response::publish::callback cb ) override
  {
    this->template call<_publish_>( std::move(req), cb, nullptr);
  }

  virtual void get_messages(request::get_messages::ptr req, response::get_messages::callback cb ) override
  {
    this->template call<_get_messages_>( std::move(req), cb, nullptr);
  }

  virtual void subscribe( request::subscribe::ptr req, response::subscribe::callback cb,
                          io_id_t, std::weak_ptr<ipubsub> ) override
  {
    this->template call<_subscribe_>( std::move(req), cb, nullptr);
  }

  virtual void describe( request::describe::ptr req, response::describe::callback cb, io_id_t ) override
  {
    this->template call<_describe_>( std::move(req), cb, nullptr);
  }
};


struct service_method_list: wfc::jsonrpc::method_list
<
  wfc::jsonrpc::target<ipubsub>,
  wfc::jsonrpc::interface_<ipubsub>,
  wfc::jsonrpc::dual_method< _publish_, request::publish_json, response::publish_json, ipubsub, &ipubsub::publish>,
  wfc::jsonrpc::dual_method< _get_messages_, request::get_messages_json, response::get_messages_json, ipubsub, &ipubsub::get_messages>,
  wfc::jsonrpc::dual_method1<
    _subscribe_,
    request::subscribe_json,
    response::subscribe_json,
    ipubsub,
    &ipubsub::subscribe
  >,
  wfc::jsonrpc::dual_method3<_describe_, request::describe_json, response::describe_json, ipubsub, &ipubsub::describe>

>
{
};

}}
