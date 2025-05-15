#pragma once

#include <comet_adapter/icomet_adapter.hpp>
#include <comet_adapter/api/publish_json.hpp>
#include <comet_adapter/api/mpublish_json.hpp>
#include <comet_adapter/api/subscribe_json.hpp>
#include <comet_adapter/api/describe_json.hpp>
#include <comet_adapter/api/load_json.hpp>
#include <comet_adapter/api/mload_json.hpp>
#include <wfc/jsonrpc.hpp>
#include <iostream>
#include <pubsub/logger.hpp>

namespace wfc{ namespace comet_adapter{

JSONRPC_TAG(publish)
JSONRPC_TAG(mpublish)
JSONRPC_TAG(subscribe)
JSONRPC_TAG(describe)
JSONRPC_TAG(load)
JSONRPC_TAG(mload)

template<typename Base>
class comet_adapter_interface: public Base
{
public:
  typedef Base super;
  typedef typename super::io_id_t io_id_t;

  virtual void publish(request::publish::ptr req, response::publish::callback cb ) override
  {
    this->template call<_publish_>( std::move(req), cb, nullptr);
  }

  virtual void mpublish(request::mpublish::ptr req, response::mpublish::callback cb ) override
  {
    this->template call<_mpublish_>( std::move(req), cb, nullptr);
  }

  virtual void subscribe( request::subscribe::ptr req, response::subscribe::callback cb,
                          io_id_t, std::weak_ptr<icomet_adapter> ) override
  {
    this->template call<_subscribe_>( std::move(req), cb, nullptr);
  }

  virtual void describe( request::describe::ptr req, response::describe::callback cb, io_id_t ) override
  {
    this->template call<_describe_>( std::move(req), cb, nullptr);
  }

  virtual void load( request::load::ptr req, response::load::callback cb ) override
  {
    this->template call<_load_>( std::move(req), cb, nullptr);
  }

  virtual void mload( request::mload::ptr req, response::mload::callback cb ) override
  {
    this->template call<_mload_>( std::move(req), cb, nullptr);
  }
};


struct service_method_list: wfc::jsonrpc::method_list
<
  wfc::jsonrpc::target<icomet_adapter>,
  wfc::jsonrpc::interface_<icomet_adapter>,
  wfc::jsonrpc::dual_method< _load_, request::load_json, response::load_json, icomet_adapter, &icomet_adapter::load>,
  wfc::jsonrpc::dual_method< _mload_, request::mload_json, response::mload_json, icomet_adapter, &icomet_adapter::mload>,
  wfc::jsonrpc::dual_method< _publish_, request::publish_json, response::publish_json, icomet_adapter, &icomet_adapter::publish>,
  wfc::jsonrpc::dual_method< _mpublish_, request::mpublish_json, response::mpublish_json, icomet_adapter, &icomet_adapter::mpublish>,
  wfc::jsonrpc::dual_method1<
    _subscribe_,
    request::subscribe_json,
    response::subscribe_json,
    icomet_adapter, icomet_adapter,
    &icomet_adapter::subscribe
  >,
  wfc::jsonrpc::dual_method3<_describe_, request::describe_json, response::describe_json, icomet_adapter, &icomet_adapter::describe>

>
{
};

}}
