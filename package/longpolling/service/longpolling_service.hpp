#pragma once

#include <longpolling/ilongpolling.hpp>
#include <longpolling/api/create_json.hpp>
#include <longpolling/api/open_json.hpp>
#include <longpolling/api/close_json.hpp>

#include <longpolling/api/longpoll_json.hpp>
#include <longpolling/api/publish_json.hpp>
#include <wfc/jsonrpc.hpp>

namespace wfc{ namespace pubsub{

JSONRPC_TAG(create)
JSONRPC_TAG(open)
JSONRPC_TAG(close)
JSONRPC_TAG(longpoll)
JSONRPC_TAG(publish)


struct longpolling_service_method_list
  : jsonrpc::method_list
    <
      jsonrpc::target<ilongpolling>,
      jsonrpc::invoke_method< _create_, request::create_json,   response::create_json, ilongpolling, &ilongpolling::create1>,
      jsonrpc::invoke_method< _open_,     request::open_json,     response::open_json,     ilongpolling, &ilongpolling::open>,
      jsonrpc::invoke_method< _close_,     request::close_json,     response::close_json,     ilongpolling, &ilongpolling::close>,
      jsonrpc::invoke_method< _longpoll_, request::longpoll_json, response::longpoll_json, ilongpolling, &ilongpolling::longpoll>,
      jsonrpc::invoke_method< _publish_,  request::publish_json,  response::publish_json,  ilongpolling, &ilongpolling::publish>
    >
{
};

}}
