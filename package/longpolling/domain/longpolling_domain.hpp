//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "longpolling_config.hpp"
#include "stat/longpolling_stat.hpp"
#include <wfc/domain_object.hpp>
#include <longpolling/ilongpolling.hpp>
#include <longpolling/auth/longpolling_auth.hpp>
#include <longpolling/hub_rpc_inflight.hpp>
#include <pubsub/ipubsub.hpp>
#include <memory>

namespace wfc{ namespace pubsub{

class longpolling;

class longpolling_domain
  : public wfc::domain_object<ilongpolling, longpolling_config, defstat>
  , public std::enable_shared_from_this<longpolling_domain>
{
  typedef wfc::domain_object<ilongpolling, longpolling_config, defstat> super;
 // class subscriber;
public:
  virtual ~longpolling_domain();
  void configure() override;
  void initialize() override;
  void start() override;
  void stop() override;

  // iinterface
  virtual void reg_io(io_id_t id, std::weak_ptr<iinterface> itf) override;
  virtual void unreg_io(io_id_t id) override;

  // ilongpolling
  virtual void create1( request::create::ptr req, response::create::callback cb ) override;
  virtual void open( request::open::ptr req, response::open::callback cb ) override;
  virtual void close( request::close::ptr req, response::close::callback cb ) override;
  virtual void longpoll( request::longpoll::ptr req, response::longpoll::callback cb ) override;
  virtual void publish( request::publish::ptr req, response::publish::callback cb ) override;

  // isubscriber
  virtual void notify( request::publish::ptr req, response::publish::callback cb ) override;


private:
  bool longpolling_fire_();
  size_t make_subscriptions_();
  size_t make_descriptions_();

  void handle_hub_message_(const topic& m);
  void on_subscribe_response_(const std::vector<std::string>& subs_channels, size_t batch_count, response::subscribe::ptr res);
  void on_describe_response_(const std::vector<std::string>& desc_channels, size_t batch_count, response::describe::ptr res);
  void reset_hub_inflight_();
private:

  typedef std::shared_ptr<longpolling> longpolling_ptr;
  std::weak_ptr<ipubsub> _wpubsub;
  longpolling_ptr _longpolling;
  longpolling_auth _auth;
  longpolling_stat _stat;
  hub_rpc_inflight _hub_inflight;
  timer_id_t _fire_timer = -1;
  timer_id_t _ping_timer = -1;
  size_t _subscribe_batch = 1000;
};

}}
