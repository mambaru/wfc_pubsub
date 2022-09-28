#pragma once

#include <comet_adapter/api/describe.hpp>
#include <comet_adapter/api/status_json.hpp>
#include <wfc/json.hpp>

namespace wfc{ namespace comet_adapter{

namespace request
{
  struct describe_json
  {
    typedef wjson::value<describe::type> meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace response
{
  struct describe_json
  {
    typedef status_json meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}

