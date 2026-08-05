#pragma once

#include <pubsub/api/ping.hpp>
#include <wfc/json.hpp>

namespace wfc{ namespace pubsub{

namespace request
{
  struct ping_json
  {
    typedef wjson::object<
      ping,
      wjson::member_list<
      >
    > meta;

    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace response
{
  JSON_NAME(uuid)
  struct ping_json
  {
    typedef wjson::object<
      ping,
      wjson::member_list<
        wjson::member<n_uuid, ping, std::string, &ping::uuid>
      >
    > meta;

    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}

