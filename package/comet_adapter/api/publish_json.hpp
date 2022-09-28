#pragma once

#include <comet_adapter/api/publish.hpp>
#include <pubsub/api/message_json.hpp>
#include <comet_adapter/api/status_json.hpp>
#include <wjson/wjson.hpp>

namespace wfc{ namespace comet_adapter{

namespace request
{
  struct publish_json
  {
    JSON_NAME(channel)
    typedef wjson::object<
      publish,
      wjson::member_list<
        wjson::member<n_channel, publish, std::string, &publish::channel>,
        wjson::base<pubsub::message_json>
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
    typedef status_json meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}
