#pragma once

#include <longpolling/ilongpolling.hpp>
#include <longpolling/icomet_subscriber.hpp>
#include <longpolling/api/create_json.hpp>
#include <longpolling/api/open_json.hpp>
#include <longpolling/api/close_json.hpp>

#include <longpolling/api/longpoll_json.hpp>
#include <longpolling/api/publish_json.hpp>
#include <longpolling/api/subscribe_json.hpp>
#include <wfc/jsonrpc.hpp>

namespace wfc{ namespace pubsub{

JSONRPC_TAG(create)
JSONRPC_TAG(open)
JSONRPC_TAG(close)
JSONRPC_TAG(longpoll)
JSONRPC_TAG(publish)
JSONRPC_TAG(subscribe)
JSONRPC_TAG2(_comet_subscriber_, "subscribe")
JSONRPC_TAG2(_comet_create_, "create")


namespace depr
{
  template<typename Base>
  class comet_subscriber: public Base
  {
  public:
    typedef Base super;
    typedef typename super::io_id_t io_id_t;

    virtual void subscribe( request::subscribe::ptr req, response::subscribe::callback cb ) override
    {
      this->template call<_comet_subscriber_>( std::move(req), cb, nullptr);
    }

    /*
    virtual void create( depr::request::create::ptr req ) override
    {
      this->template call<_comet_create_>( std::move(req), nullptr, nullptr);
    }*/
  };
}

struct longpolling_service_method_list
  : jsonrpc::method_list
    <
      jsonrpc::target<ilongpolling>,
      jsonrpc::interface_<depr::icomet_subscriber>,
      jsonrpc::invoke_method< _create_, depr::request::create_json,   depr::response::create_json, ilongpolling, &ilongpolling::create_depr>,
      jsonrpc::invoke_method< _open_,     request::open_json,     response::open_json,     ilongpolling, &ilongpolling::open>,
      jsonrpc::invoke_method< _close_,     request::close_json,     response::close_json,     ilongpolling, &ilongpolling::close>,
      jsonrpc::invoke_method< _longpoll_, request::longpoll_json, response::longpoll_json, ilongpolling, &ilongpolling::longpoll>,
      jsonrpc::invoke_method< _publish_,  request::publish_json,  response::publish_json,  ilongpolling, &ilongpolling::publish>,
      jsonrpc::invoke_method1< _subscribe_,
        request::longpoll_json, depr::response::subscribe_json,
        ilongpolling, depr::icomet_subscriber, &ilongpolling::subscribe>,
      jsonrpc::call_method< _comet_subscriber_, depr::request::subscribe_json, depr::response::subscribe_json>/*,
      jsonrpc::call_method< _comet_create_, depr::request::create_json, depr::response::create_json>
      */
    >
{
};

}}
