#pragma once

#include <pubsub/api/subscribe.hpp>
#include <message_queue/subscribe_params_json.hpp>
#include <wjson/wjson.hpp>

namespace wfc{ namespace pubsub{

namespace request
{
  struct subscribe_json
  {
    JSON_NAME(channels)
    typedef wjson::object<
      subscribe,
      wjson::member_list<
        wjson::member<n_channels, subscribe, subscribe::params_list_t, &subscribe::channels,
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
  struct subscribe_json
  {
    JSON_NAME(messages)

    typedef wjson::object<
      subscribe,
      wjson::member_list<
        wjson::member<n_messages, subscribe, topic::topic_list_t, &subscribe::messages, topic_list_json>
      >
    > meta;

    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}
