#pragma once

#include <pubsub/api/publish.hpp>
#include <message_queue/topic_json.hpp>
#include <wfc/json.hpp>

namespace wfc{ namespace pubsub{

namespace request
{
  struct publish_json
  {
    JSON_NAME(messages)

    typedef wjson::object<
      publish,
      wjson::member_list<
        wjson::member<n_messages, publish, topic::topic_list_t, &publish::messages, topic_list_json>
      >
    > meta;

    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace response
{
  struct publish_json
  {
    typedef wjson::object<
      publish,
      wjson::member_list<
      >
    > meta;

    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}
