#pragma once

#include <longpolling/api/open.hpp>
#include <longpolling/agent_params_json.hpp>
#include <longpolling/channel_params_json.hpp>
#include <wjson/json.hpp>

namespace wfc{ namespace pubsub{

namespace request
{
  struct open_json
  {
    JSON_NAME(channels)
    typedef wjson::object<
      open,
      wjson::member_list<
        wjson::base<agent_params_json>,
        wjson::member<
          n_channels, open, channel_params::channel_list_t, &open::channels,
          channel_params_list_json
        >
      >
    > meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace response
{
  struct open_json
  {
    typedef wjson::object<
      open,
      wjson::member_list<
      >
    > meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}
