//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "comet_adapter_config.hpp"
#include <wfc/domain_object.hpp>
#include <comet_adapter/icomet_adapter.hpp>
#include <pubsub/ipubsub.hpp>
#include <memory>

namespace wfc{ namespace comet_adapter{

class comet_adapter_domain
  : public wfc::domain_object<icomet_adapter, comet_adapter_config>
  , public std::enable_shared_from_this<comet_adapter_domain>
{
public:
  comet_adapter_domain();
  virtual void reg_io(io_id_t io_id, std::weak_ptr<iinterface> itf) override;
  virtual void unreg_io(io_id_t io_id) override;
  virtual void configure() override;
  virtual void initialize() override;
  virtual void publish( request::publish::ptr req, response::publish::callback cb ) override;
  virtual void mpublish( request::mpublish::ptr req, response::mpublish::callback cb ) override;
  virtual void subscribe( request::subscribe::ptr req, response::subscribe::callback cb,
                          io_id_t io_id, std::weak_ptr<icomet_adapter> ) override;
  virtual void describe( request::describe::ptr req, response::describe::callback cb, io_id_t io_id ) override;

  virtual void load( request::load::ptr req, response::load::callback cb ) override;
  virtual void mload( request::mload::ptr req, response::mload::callback cb ) override;

private:
  typedef std::weak_ptr<pubsub::ipubsub> wpubsub_ptr;
  typedef std::shared_ptr<pubsub::ipubsub> pubsub_ptr;
  wpubsub_ptr _target;
  typedef std::mutex mutex_type;
  std::map<io_id_t, pubsub_ptr> _adapter_map;
  mutable mutex_type _mutex;
};

}}
