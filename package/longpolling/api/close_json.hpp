#pragma once

#include <longpolling/api/close.hpp>
#include <wjson/json.hpp>

namespace wfc{ namespace pubsub{

namespace request
{
  struct close_json
  {
    JSON_NAME(uuid)
    JSON_NAME(confirm)
    JSON_NAME(channels)
    typedef wjson::object<
      close,
      wjson::member_list<
        wjson::member<n_uuid, close, std::string, &close::uuid>,
        wjson::member<n_confirm, close, bool, &close::confirm>,
        wjson::member<n_channels, close, std::vector<std::string>, &close::channels, wjson::vector_of_strings<> >
      >
    > meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace response
{
  struct close_json
  {
    typedef wjson::object<
      close,
      wjson::member_list<
      >
    > meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}
