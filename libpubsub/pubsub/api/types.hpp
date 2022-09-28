//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2011, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <memory>

namespace wfc{ namespace pubsub{

typedef size_t subscriber_id_t;
typedef std::string identity_t;
typedef std::string content_t;
typedef std::unique_ptr<content_t> content_ptr;
typedef time_t cursor_t;
typedef long key_t;
typedef std::unique_ptr<key_t> key_ptr;

}}
