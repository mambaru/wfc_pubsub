#pragma once

#include <longpolling/agent_map_options.hpp>
#include <wjson/wjson.hpp>

namespace wfc{ namespace pubsub{

struct agent_map_options_json
{
  JSON_NAME(hash_size)

  typedef wjson::object<
    agent_map_options,
    wjson::member_list<
      wjson::member< n_hash_size, agent_map_options, size_t, &agent_map_options::hash_size >
    >,
    wjson::strict_mode
  > meta;

  typedef meta::serializer serializer;
  typedef meta::target target;
  typedef meta::member_list member_list;
};

}}
