//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#include "rocksdb_factory.hpp"
#include "../logger.hpp"
#include "merge_operator.hpp"

#include <rocksdb/db.h>
#include <rocksdb/env.h>
#include <rocksdb/options.h>
#include <rocksdb/merge_operator.h>
#include <rocksdb/compaction_filter.h>
#include <rocksdb/utilities/options_util.h>
#include <rocksdb/utilities/db_ttl.h>
#include <boost/filesystem.hpp>

namespace wfc{ namespace pubsub{

rocksdb_factory::~rocksdb_factory()
{

}

bool rocksdb_factory::configure(const rocksdb_options& opt)
{
  if ( opt.path.empty() )
    return false;

  std::lock_guard<mutex_type> lk(_mutex);

  if ( !::boost::filesystem::exists(opt.path) )
  {
    ::boost::system::error_code ec;
    ::boost::filesystem::create_directory(opt.path, ec);
    if (ec)
    {
      PUBSUB_LOG_FATAL("Create directory fail:'" << opt.path << "'" << ec.message() );
      return false;
    }
  }

  _opt = opt;
  _env.env = ::rocksdb::Env::Default();
  _env.options.create_if_missing = true;
  _env.options.env = _env.env;

  if ( !_opt.ini.empty() )
  {
    auto status = ::rocksdb::LoadOptionsFromFile( ::rocksdb::ConfigOptions(), _opt.ini, &(_env.options), &(_env.cdf) );
    if ( !status.ok() )
    {
      PUBSUB_LOG_FATAL("rocksdb_factory::initialize: " << status.ToString());
      return false;
    }
  }
  else
  {
    _env.cdf.push_back( rocksdb_env::CFD() );
    _env.options = ::rocksdb::Options();
  }

  _env.cdf[0].options.merge_operator = std::make_shared<merge_operator>();

  return true;
}


rocksdb_factory::rocksdb_ptr rocksdb_factory::create(time_t ttl) const
{
  auto db = std::make_shared<rocksdb>();
  std::lock_guard<mutex_type> lk(_mutex);
  if ( db->create(ttl, _opt, _env) )
    return db;
  db.reset();
  return db;
}

}}
