#pragma once

#include <longpolling/api/create.hpp>
#include <longpolling/agent_options_json.hpp>
#include <longpolling/channel_params_json.hpp>
#include <wjson/json.hpp>

namespace wfc{ namespace pubsub{

namespace request
{
  struct create_json
  {
    JSON_NAME(channels)
    typedef wjson::object<
      create,
      wjson::member_list<
        wjson::base<agent_options_json>,
        wjson::member<
          n_channels, create, channel_params::channel_list_t, &create::channels,
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
  struct create_json
  {
    JSON_NAME(uuid)
    typedef wjson::object<
      create,
      wjson::member_list<
        wjson::member<n_uuid, create, std::string, &create::uuid>
      >
    > meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace depr{
  namespace request{
    using create_json = ::wfc::pubsub::request::create_json;

  }

  namespace response{
    struct create_json
    {
      typedef wjson::member_value<create, create, std::string, &create::uuid> meta;
      typedef meta::target     target;
      typedef meta::serializer serializer;
    };
  }
}
}}
