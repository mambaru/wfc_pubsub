#pragma once

#include <comet_adapter/api/status.hpp>
#include <wjson/wjson.hpp>

namespace wfc{ namespace comet_adapter{

struct status_json
{
  JSON_NAME(ready)
  JSON_NAME(forbidden)
  JSON_NAME(bad_gateway)
  JSON_NAME(not_found)
  JSON_NAME(internal_error)
  JSON_NAME(not_support)
  JSON_NAME(wait)

  typedef wjson::member_list<
    wjson::enum_value<n_ready, status::type, status::ready>,
    wjson::enum_value<n_forbidden, status::type, status::forbidden>,
    wjson::enum_value<n_bad_gateway, status::type, status::bad_gateway>,
    wjson::enum_value<n_not_found, status::type, status::not_found>,
    wjson::enum_value<n_internal_error, status::type, status::internal_error>,
    wjson::enum_value<n_not_support, status::type, status::not_support>,
    wjson::enum_value<n_wait, status::type, status::wait>
  > status_enum_list;

  typedef wjson::enumerator< status::type, status_enum_list > meta;
  typedef meta::target target;
  typedef meta::serializer serializer;

};

}}

