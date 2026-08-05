//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2023
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <string>

namespace wfc{

struct http_config
{
  std::string content_path;
  std::string jsonrpc_path;
  std::string target;
};

}

