//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2013, 2014, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <pubsub/api/message.hpp>
#include <pubsub/api/actions_json.hpp>

namespace wfc{ namespace pubsub{

struct message_json
{
  JSON_NAME(action)
  JSON_NAME(limit)
  JSON_NAME(lifetime)
  JSON_NAME(birthtime)
  JSON_NAME(cursor)
  JSON_NAME(identity)
  JSON_NAME(key)
  JSON_NAME(content)

  typedef
    wjson::object<
      message,
      wjson::member_list<
        wjson::member<n_action, message, actions, &message::action, actions_json >,
        wjson::member<n_limit, message, size_t, &message::limit >,
        wjson::member<n_lifetime, message, time_t, &message::lifetime >,
        wjson::member<n_cursor, message, cursor_t, &message::cursor >,
        wjson::member<n_identity, message, identity_t, &message::identity >,
        wjson::member<n_key, message, key_t, &message::key >,
        wjson::member<n_content, message, content_ptr, &message::content,  wjson::pointer< content_ptr, wjson::raw_value<content_t> > >,
        wjson::member<n_birthtime, message, time_t, &message::birthtime >
      >
    > meta;
  typedef meta::target target;
  typedef meta::serializer serializer;
  typedef meta::member_list member_list;

};

}}
