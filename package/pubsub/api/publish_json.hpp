#pragma once

#include <pubsub/api/publish.hpp>
#include <wfc/json.hpp>

namespace wfc{ namespace pubsub{

namespace request
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
