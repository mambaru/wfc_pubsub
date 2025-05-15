#pragma once

#include <longpolling/api/subscribe.hpp>
#include <longpolling/api/create.hpp>

namespace wfc{ namespace pubsub{

namespace depr{
  struct icomet_subscriber: iinterface
  {
    virtual ~icomet_subscriber(){}
    virtual void subscribe( request::subscribe::ptr req, response::subscribe::callback cb ) = 0;
    //virtual void create( depr::request::create::ptr res) =0;
  };
}

}}
