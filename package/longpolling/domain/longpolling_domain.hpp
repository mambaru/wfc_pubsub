//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "longpolling_config.hpp"
#include <wfc/domain_object.hpp>
#include <longpolling/ilongpolling.hpp>
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
  virtual void subscribe(
    request::longpoll::ptr req, depr::response::subscribe::callback cb,
    io_id_t io_id, std::weak_ptr<depr::icomet_subscriber>) override;

  // isubscriber
  virtual void notify( request::publish::ptr req, response::publish::callback cb ) override;
  virtual void create_depr( depr::request::create::ptr req, depr::response::create::callback cb) override;


private:
  bool longpolling_fire_();
  size_t make_subscriptions_();
  size_t make_descriptions_();
private:

  auto pop_for_(bool (longpolling::*poll)(std::vector<std::string>* cl, size_t limit));
  //typedef std::shared_ptr<subscriber> subscriber_ptr;
  typedef std::shared_ptr<longpolling> longpolling_ptr;
  std::weak_ptr<ipubsub> _wpubsub;
  longpolling_ptr _longpolling;
  //subscriber_ptr _subscriber;
  timer_id_t _fire_timer = -1;
  timer_id_t _ping_timer = -1;
  size_t _subscribe_batch = 1000;
  std::vector<wfc::value_meter> _fire_meters;
};

}}
