//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2023
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once

#include "http_config.hpp"
#include <wfc/json.hpp>

namespace wfc{

JSON_NAME(content_path)
JSON_NAME(jsonrpc_path)
JSON_NAME(target)

struct http_config_json
{
  typedef wjson::object<
    http_config,
    wjson::member_list<
      wjson::member< n_content_path, http_config, std::string, &http_config::content_path>,
      wjson::member< n_jsonrpc_path, http_config, std::string, &http_config::jsonrpc_path>,
      wjson::member< n_target, http_config, std::string, &http_config::target>
    >,
    wjson::strict_mode
  > meta;

  typedef meta::serializer serializer;
  typedef meta::target target;
  typedef meta::member_list member_list;
};

}

