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

template<typename Base>
class subscriber_interface: public Base
{
public:
  typedef Base super;
  typedef typename super::io_id_t io_id_t;

  virtual void notify(request::publish::ptr req, response::publish::callback cb ) override
  {
    this->template call<_publish_>( std::move(req), cb, nullptr);
  }
};


struct pubsub_service_method_list: wfc::jsonrpc::method_list
<
  wfc::jsonrpc::target<ipubsub>,
  wfc::jsonrpc::interface_<isubscriber>,
  wfc::jsonrpc::dual_method< _publish_, request::publish_json, response::publish_json, ipubsub, &ipubsub::publish>,
  wfc::jsonrpc::invoke_method< _get_messages_, request::get_messages_json, response::get_messages_json, ipubsub, &ipubsub::get_messages>,
  wfc::jsonrpc::invoke_method1<
    _subscribe_,
    request::subscribe_json,
    response::subscribe_json,
    ipubsub, isubscriber,
    &ipubsub::subscribe
  >,
  wfc::jsonrpc::invoke_method3<_describe_, request::describe_json, response::describe_json, ipubsub, &ipubsub::describe>,
  wfc::jsonrpc::invoke_method3<_ping_, request::ping_json, response::ping_json, ipubsub, &ipubsub::ping>

>
{
};

}}
