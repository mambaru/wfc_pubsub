//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once

#include <pubsub/api/subscribe_params.hpp>
#include <wjson/json.hpp>

namespace wfc{ namespace pubsub{

struct subscribe_params_json
{
  JSON_NAME(channel)
  JSON_NAME(cursor)
  JSON_NAME(limit)

  typedef wjson::object<
    subscribe_params,
    wjson::member_list<
      wjson::member<n_channel, subscribe_params, std::string, &subscribe_params::channel>,
      wjson::member<n_cursor, subscribe_params, cursor_t, &subscribe_params::cursor>,
      wjson::member<n_limit, subscribe_params, size_t, &subscribe_params::limit>
    >
  > type;

  typedef type::serializer serializer;
  typedef type::target target;
  typedef type::member_list member_list;
};

}}


