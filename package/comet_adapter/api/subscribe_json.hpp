#pragma once

#include <comet_adapter/api/subscribe.hpp>
#include <comet_adapter/api/status_json.hpp>
#include <wfc/json.hpp>

namespace wfc{ namespace comet_adapter{

namespace request
{
  struct subscribe_json
  {
    typedef wjson::value<subscribe::type> meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace response
{
  struct subscribe_json
  {
    typedef status_json meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}
