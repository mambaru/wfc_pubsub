#pragma once

#include <pubsub/api/subscribe.hpp>
#include <wfc/json.hpp>

namespace wfc{ namespace pubsub{

namespace request
{
  struct subscribe_json
  {
    typedef wjson::object<
      subscribe,
      wjson::member_list<
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
    typedef wjson::object<
      subscribe,
      wjson::member_list<
      >
    > meta;

    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}
