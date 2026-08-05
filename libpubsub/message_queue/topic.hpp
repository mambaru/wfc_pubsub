//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2023
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <message_queue/message.hpp>
#include <string>
#include <algorithm>
#include <deque>

namespace wfc{ namespace pubsub{

struct topic
  : message
{
  typedef std::vector<topic> topic_list_t;
  std::string channel;

  static topic copy(const topic& m)
  {
    topic t;
    t.channel = m.channel;
    static_cast<message&>(t) = message::copy(m);
    return t;
  }

  static topic_list_t copy_list(const topic_list_t& tl)
  {
    return copy_list( tl.begin(), tl.end() );
  }

  template<typename Itr>
  static topic_list_t copy_list(Itr beg, Itr end)
  {
    topic_list_t tlc;
    std::transform(beg, end, std::back_inserter(tlc), &topic::copy);
    return tlc;
  }



};

}}

