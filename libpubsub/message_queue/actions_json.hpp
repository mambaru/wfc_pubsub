//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2013, 2014, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <message_queue/actions.hpp>
#include <wjson/wjson.hpp>


namespace wfc{ namespace pubsub{

struct actions_json
{
  JSON_NAME(publish)
  JSON_NAME(modify)
  JSON_NAME(update)
  JSON_NAME(replace)
  JSON_NAME(remove)

  typedef wjson::member_list<
    wjson::enum_value<n_publish, actions, actions::publish>,
    wjson::enum_value<n_modify, actions, actions::modify>,
    wjson::enum_value<n_update, actions, actions::update>,
    wjson::enum_value<n_replace, actions, actions::replace>,
    wjson::enum_value<n_remove, actions, actions::remove>
  > actions_enum_list;

  typedef wjson::enumerator< actions, actions_enum_list > meta;
  typedef meta::target target;
  typedef meta::serializer serializer;
};

}}
