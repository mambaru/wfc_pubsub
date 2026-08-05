
#pragma once

#include <longpolling/api/longpoll.hpp>
#include <longpolling/agent_params_json.hpp>
#include <message_queue/topic_json.hpp>

#include <wjson/json.hpp>

namespace wfc{ namespace pubsub{

namespace request
{
  struct longpoll_json
  {
    typedef wjson::object<
      longpoll,
      wjson::member_list<
        wjson::base<agent_params_json>
      >
    > meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace response
{
  struct longpoll_json
  {
    JSON_NAME(messages)
    typedef wjson::object<
      longpoll,
      wjson::member_list<
        wjson::member<n_messages, longpoll, topic::topic_list_t, &longpoll::messages, topic_list_json>
      >
    > meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}
