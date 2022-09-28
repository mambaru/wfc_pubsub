//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once
#include <wfc/iinterface.hpp>
#include <comet_adapter/api/publish.hpp>
#include <comet_adapter/api/mpublish.hpp>
#include <comet_adapter/api/subscribe.hpp>
#include <comet_adapter/api/describe.hpp>
#include <comet_adapter/api/load.hpp>
#include <comet_adapter/api/mload.hpp>

namespace wfc{ namespace comet_adapter{

struct icomet_adapter: iinterface
{
  virtual ~icomet_adapter(){}
  virtual void publish( request::publish::ptr req, response::publish::callback cb ) = 0;
  virtual void mpublish( request::mpublish::ptr req, response::mpublish::callback cb ) = 0;
  virtual void subscribe( request::subscribe::ptr req, response::subscribe::callback cb,
                          io_id_t io_id, std::weak_ptr<icomet_adapter> ) = 0;
  virtual void describe( request::describe::ptr req, response::describe::callback cb, io_id_t io_id ) = 0;

  virtual void load( request::load::ptr req, response::load::callback cb ) = 0;
  virtual void mload( request::mload::ptr req, response::mload::callback cb ) = 0;

};

}}

