
//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2023
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <message_queue/topic.hpp>
#include <message_queue/message_json.hpp>
#include <wjson/wjson.hpp>

namespace wfc{ namespace pubsub{

struct topic_json
{
  JSON_NAME(channel)

  typedef
    wjson::object<
      topic,
      wjson::member_list<
        wjson::member<n_channel, topic, std::string, &topic::channel>,
        wjson::base<message_json>
      >
    > meta;
  typedef meta::target target;
  typedef meta::serializer serializer;
  typedef meta::member_list member_list;
};

typedef wjson::vector_of<topic_json> topic_list_json;

}}
