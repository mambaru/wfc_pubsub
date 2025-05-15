

#pragma once

#include <comet_adapter/api/load.hpp>
#include <comet_adapter/api/status_json.hpp>
#include <message_queue/subscribe_params_json.hpp>
#include <message_queue/message_json.hpp>
#include <wjson/json.hpp>

namespace wfc{ namespace comet_adapter{

namespace request
{
  struct load_json
  {
    typedef wjson::object<
      load,
      wjson::member_list<
        wjson::base<pubsub::subscribe_params_json>
      >
    > meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace response
{
  struct load_json
  {
    JSON_NAME(status)
    JSON_NAME(channel)
    JSON_NAME(messages)

    typedef wjson::object<
      load,
      wjson::member_list<
        wjson::member<n_status, load, status::type, &load::load_status, status_json>,
        wjson::member<n_channel, load, std::string, &load::channel>,
        wjson::member<n_messages, load, load::message_list, &load::messages,
          wjson::vector_of<pubsub::message_json>
        >
      >
    > meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}
