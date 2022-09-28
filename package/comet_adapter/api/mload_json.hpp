#pragma once

#include <comet_adapter/api/mload.hpp>
#include <comet_adapter/api/load_json.hpp>
#include <comet_adapter/api/status_json.hpp>
#include <pubsub/api/subscribe_params_json.hpp>
#include <pubsub/api/message_json.hpp>
#include <wjson/json.hpp>

namespace wfc{ namespace comet_adapter{

namespace request
{
  struct mload_json
  {
    typedef wjson::vector_of< pubsub::subscribe_params_json > meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace response
{
  struct mload_json
  {
    typedef wjson::vector_of< load_json > meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}
