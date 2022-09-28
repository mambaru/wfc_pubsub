#pragma once

#include <comet_adapter/api/mpublish.hpp>
#include <comet_adapter/api/publish_json.hpp>
#include <comet_adapter/api/status_json.hpp>
#include <wfc/json.hpp>

namespace wfc{ namespace comet_adapter{

namespace request
{
  struct mpublish_json
  {
    typedef wjson::vector_of<publish_json> meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace response
{
  struct mpublish_json
  {
    typedef status_json meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}
