#pragma once

#include <pubsub/api/describe.hpp>
#include <wfc/json.hpp>

namespace wfc{ namespace pubsub{

namespace request
{
  struct describe_json
  {
    JSON_NAME(channels)
    typedef wjson::object<
      describe,
      wjson::member_list<
        wjson::member<n_channels, describe, std::vector<std::string>, &describe::channels,
          wjson::vector_of< wjson::value<std::string> > >
      >
    > meta;

    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace response
{
  struct describe_json
  {
    typedef wjson::object<
      describe,
      wjson::member_list<
      >
    > meta;

    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}

