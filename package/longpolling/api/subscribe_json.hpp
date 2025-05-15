
#pragma once

#include <longpolling/api/subscribe.hpp>
#include <message_queue/topic_json.hpp>

#include <wjson/json.hpp>

namespace wfc{ namespace pubsub{ namespace depr{

namespace request
{
  struct subscribe_json
  {
    typedef topic_list_json meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

namespace response
{
  struct subscribe_json
  {
    typedef topic_list_json meta;
    typedef meta::target     target;
    typedef meta::serializer serializer;
  };
}

}}}

