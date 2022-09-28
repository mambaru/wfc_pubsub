//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "pubsub_config.hpp"
#include <wfc/domain_object.hpp>
#include <pubsub/pubsub_mt.hpp>
#include <pubsub/ipubsub.hpp>
#include <memory>

namespace wfc{ namespace pubsub{

class pubsub_domain
  : public wfc::domain_object<ipubsub, pubsub_config>
  , public std::enable_shared_from_this<pubsub_domain>
{
public:
  pubsub_domain();
  virtual void unreg_io(io_id_t io_id) override;
  virtual void configure() override;
  virtual void reconfigure() override;
  virtual void start() override;
  virtual void stop() override;
  virtual void publish( request::publish::ptr req, response::publish::callback cb ) override;
  virtual void get_messages(request::get_messages::ptr req, response::get_messages::callback cb ) override;

  virtual void subscribe( request::subscribe::ptr req, response::subscribe::callback cb,
                          io_id_t io_id, std::weak_ptr<ipubsub> ) override;
  virtual void describe( request::describe::ptr req, response::describe::callback cb, io_id_t io_id) override;
private:
  std::shared_ptr<pubsub_mt> _pubsub;
  std::atomic_bool _debug_reset;
};

}}
