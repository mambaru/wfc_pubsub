#include <fas/testing.hpp>
#include <pubsub/rocksdb/rocksdb.hpp>
#include <pubsub/rocksdb/rocksdb_factory.hpp>
#include <pubsub/rocksdb/multi_rocksdb.hpp>
#include <message_queue/message.hpp>
#include <message_queue/types.hpp>

#include <boost/filesystem.hpp>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <vector>

namespace{

std::string make_db_path(const std::string& tag)
{
  namespace fs = boost::filesystem;
  fs::path root = fs::temp_directory_path() / "wfc_pubsub_rocksdb_tests";
  fs::create_directories(root);
  fs::path dir = root / (tag + "_" + std::to_string(::getpid()) + "_" + std::to_string(std::rand()));
  fs::create_directories(dir);
  return dir.string();
}

void remove_db_path(const std::string& path)
{
  boost::system::error_code ec;
  boost::filesystem::remove_all(path, ec);
}

wfc::pubsub::message make_msg(
  const std::string& id,
  wfc::pubsub::cursor_t cursor,
  const std::string& content_json,
  time_t lifetime = 3600,
  size_t limit = 100)
{
  wfc::pubsub::message m;
  m.identity = id;
  m.cursor = cursor;
  m.lifetime = lifetime;
  m.limit = limit;
  m.birthtime = std::time(nullptr);
  // content — raw JSON (wjson::raw_value)
  m.content = std::make_unique<wfc::pubsub::content_t>(content_json);
  return m;
}

UNIT(rocksdb_get_messages, "")
{
  using namespace fas::testing;

  const std::string path = make_db_path("get_messages");
  wfc::pubsub::rocksdb_options opt;
  opt.path = path;
  opt.ttl = {3600};

  wfc::pubsub::rocksdb_factory factory;
  t << equal<assert, bool>(factory.configure(opt), true) << FAS_FL;
  auto db = factory.create(3600);
  t << is_true<assert>(db != nullptr) << FAS_FL;
  if ( db == nullptr )
  {
    remove_db_path(path);
    return;
  }

  std::vector<wfc::pubsub::message> ml;
  t << equal<expect, bool>(db->get_messages(&ml, "ch1", 0, 10), false) << FAS_FL;
  t << equal<expect, size_t>(ml.size(), 0) << FAS_FL;

  db->push("ch1", make_msg("id1", 1, std::string("\"c1\"")));
  db->push("ch1", make_msg("id2", 2, std::string("\"c2\"")));
  db->push("ch2", make_msg("id3", 1, std::string("\"c3\"")));

  ml.clear();
  t << equal<assert, bool>(db->get_messages(&ml, "ch1", 0, 10), true) << FAS_FL;
  t << equal<assert, size_t>(ml.size(), 2) << FAS_FL;
  if ( ml.size() >= 2 )
  {
    t << equal<expect, std::string>(ml[0].identity, "id1") << FAS_FL;
    t << equal<expect, std::string>(ml[1].identity, "id2") << FAS_FL;
    t << is_true<expect>(ml[0].content != nullptr && *ml[0].content == "\"c1\"") << FAS_FL;
    t << is_true<expect>(ml[1].content != nullptr && *ml[1].content == "\"c2\"") << FAS_FL;
  }

  ml.clear();
  t << equal<assert, bool>(db->get_messages(&ml, "ch2", 0, 10), true) << FAS_FL;
  t << equal<expect, size_t>(ml.size(), 1) << FAS_FL;

  ml.clear();
  t << equal<expect, bool>(db->get_messages(&ml, "missing", 0, 10), false) << FAS_FL;

  db->close();
  remove_db_path(path);
}

UNIT(rocksdb_get_messages_cursor_limit, "")
{
  using namespace fas::testing;

  const std::string path = make_db_path("cursor_limit");
  wfc::pubsub::rocksdb_options opt;
  opt.path = path;
  opt.ttl = {3600};

  wfc::pubsub::rocksdb_factory factory;
  t << equal<assert, bool>(factory.configure(opt), true) << FAS_FL;
  auto db = factory.create(3600);
  t << is_true<assert>(db != nullptr) << FAS_FL;
  if ( db == nullptr )
  {
    remove_db_path(path);
    return;
  }

  for ( wfc::pubsub::cursor_t c = 1; c <= 5; ++c )
    db->push("ch", make_msg("id" + std::to_string(c), c, "\"" + std::to_string(c) + "\""));

  std::vector<wfc::pubsub::message> ml;
  t << equal<assert, bool>(db->get_messages(&ml, "ch", 3, 100), true) << FAS_FL;
  t << equal<assert, size_t>(ml.size(), 3) << FAS_FL;
  if ( ml.size() >= 3 )
  {
    t << equal<expect, wfc::pubsub::cursor_t>(ml[0].cursor, 3) << FAS_FL;
    t << equal<expect, wfc::pubsub::cursor_t>(ml[1].cursor, 4) << FAS_FL;
    t << equal<expect, wfc::pubsub::cursor_t>(ml[2].cursor, 5) << FAS_FL;
  }

  ml.clear();
  t << equal<assert, bool>(db->get_messages(&ml, "ch", 1, 2), true) << FAS_FL;
  t << equal<assert, size_t>(ml.size(), 2) << FAS_FL;
  if ( ml.size() >= 2 )
  {
    t << equal<expect, wfc::pubsub::cursor_t>(ml[0].cursor, 4) << FAS_FL;
    t << equal<expect, wfc::pubsub::cursor_t>(ml[1].cursor, 5) << FAS_FL;
  }

  db->close();
  remove_db_path(path);
}

UNIT(multi_rocksdb_get_messages, "")
{
  using namespace fas::testing;

  const std::string path = make_db_path("multi");
  wfc::pubsub::rocksdb_options opt;
  opt.path = path;
  opt.ttl = {60, 3600};

  wfc::pubsub::multi_rocksdb multi;
  t << equal<assert, bool>(multi.configure(true, opt), true) << FAS_FL;

  std::vector<wfc::pubsub::message> ml;
  t << equal<expect, bool>(multi.get_messages(&ml, "ch1", 0, 10), false) << FAS_FL;

  multi.push("ch1", make_msg("a", 1, std::string("\"ca\""), 3600));
  multi.push("ch1", make_msg("b", 2, std::string("\"cb\""), 60));
  multi.push("ch2", make_msg("c", 1, std::string("\"cc\""), 3600));

  ml.clear();
  t << equal<assert, bool>(multi.get_messages(&ml, "ch1", 0, 10), true) << FAS_FL;
  t << equal<assert, size_t>(ml.size(), 2) << FAS_FL;

  ml.clear();
  t << equal<assert, bool>(multi.get_messages(&ml, "ch2", 0, 10), true) << FAS_FL;
  t << equal<expect, size_t>(ml.size(), 1) << FAS_FL;

  t << equal<expect, bool>(multi.has("ch1"), true) << FAS_FL;
  t << equal<expect, bool>(multi.has("ch2"), true) << FAS_FL;
  t << equal<expect, bool>(multi.has("never-pushed"), false) << FAS_FL;

  multi.close();
  remove_db_path(path);
}

}

BEGIN_SUITE(rocksdb_suite, "")
  ADD_UNIT(rocksdb_get_messages)
  ADD_UNIT(rocksdb_get_messages_cursor_limit)
  ADD_UNIT(multi_rocksdb_get_messages)
END_SUITE(rocksdb_suite)
