

//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2013, 2014, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <longpolling/channel_params.hpp>
#include <wjson/wjson.hpp>

namespace wfc{ namespace pubsub{

struct channel_params_json
{
  JSON_NAME(channel)
  JSON_NAME(min_size)
  JSON_NAME(max_size)
  JSON_NAME(cursor)

  typedef wjson::object<
      channel_params,
      wjson::member_list<
        wjson::member<n_channel,  channel_params, std::string, &channel_params::channel>,
        wjson::member<n_min_size, channel_params, size_t,      &channel_params::min_size>,
        wjson::member<n_max_size, channel_params, size_t,      &channel_params::max_size>,
        wjson::member<n_cursor,   channel_params, cursor_t,    &channel_params::cursor>
      >
  > meta;

  typedef meta::target target;
  typedef meta::member_list member_list;
  typedef meta::serializer serializer;
};

typedef wjson::vector_of<channel_params_json> channel_params_list_json;

}}
