#pragma once

#include <longpolling/publish_options.hpp>
#include <wjson/wjson.hpp>

namespace wfc{ namespace pubsub{

struct publish_options_json
{
  JSON_NAME(enabled)
  JSON_NAME(channel_prefixes)

  typedef wjson::object<
    publish_options,
    wjson::member_list<
      wjson::member<n_enabled, publish_options, bool, &publish_options::enabled>,
      wjson::member< n_channel_prefixes, publish_options, std::vector<std::string>, &publish_options::channel_prefixes, wjson::vector_of_strings<> >
    >,
    wjson::strict_mode
  > meta;

  typedef meta::serializer serializer;
  typedef meta::target target;
  typedef meta::member_list member_list;
};

}}
