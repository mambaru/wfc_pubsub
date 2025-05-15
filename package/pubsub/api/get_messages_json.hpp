#pragma once

#include <pubsub/api/get_messages.hpp>
#include <message_queue/subscribe_params_json.hpp>
#include <message_queue/topic_json.hpp>

#include <wfc/json.hpp>

namespace wfc{ namespace pubsub{

namespace request
{
  struct get_messages_json
  {
    JSON_NAME(channels)
    typedef wjson::object<
      get_messages,
      wjson::member_list<
        wjson::member<n_channels, get_messages, get_messages::params_list_t, &get_messages::channels,
          subscribe_params_list_json
        >
      >
    > meta;

    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace response
{
  struct get_messages_json
  {
    JSON_NAME(messages)

    typedef wjson::object<
      get_messages,
      wjson::member_list<
        wjson::member<n_messages, get_messages, topic::topic_list_t, &get_messages::messages, topic_list_json>
      >
    > meta;

    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}
