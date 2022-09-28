//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once

#include "comet_adapter_config.hpp"
#include <wfc/json.hpp>

namespace wfc{ namespace comet_adapter{

struct comet_adapter_config_json
{
  JSON_NAME(target)

  typedef wjson::object<
    comet_adapter_config,
    wjson::member_list<
      wjson::member< n_target, comet_adapter_config, std::string, &comet_adapter_config::target>
    >,
    wjson::strict_mode
  > meta;

  typedef meta::serializer serializer;
  typedef meta::target target;
  typedef meta::member_list member_list;
};

}}
