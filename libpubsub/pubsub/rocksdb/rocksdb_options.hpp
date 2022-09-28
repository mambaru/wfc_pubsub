//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <vector>
#include <string>
#include <ctime>

namespace wfc{ namespace pubsub{

struct rocksdb_options
{
  std::string path;
  std::string wal_path;
  std::string ini;
  std::vector<time_t> ttl;
};

}}




