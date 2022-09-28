#pragma once

#include <pubsub/api/get_messages.hpp>
#include <wfc/json.hpp>

namespace wfc{ namespace pubsub{

namespace request
{
  struct get_messages_json
  {
    typedef wjson::object<
      get_messages,
      wjson::member_list<
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
    typedef wjson::object<
      get_messages,
      wjson::member_list<
      >
    > meta;

    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}
