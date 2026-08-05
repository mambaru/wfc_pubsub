#include <fas/testing.hpp>
#include <longpolling/auth/session_store.hpp>

namespace{

UNIT(session_store1, "")
{
  using namespace fas::testing;

  wfc::pubsub::session_store store;
  store.upsert(1001, "sess1", 3600);
  t << equal<assert, bool>(store.known(1001, "sess1"), true) << FAS_FL;
  t << equal<assert, bool>(store.known(1001, "sess2"), false) << FAS_FL;
  t << equal<assert, bool>(store.oid_known(1001), true) << FAS_FL;
  t << equal<assert, bool>(store.oid_known(1002), false) << FAS_FL;
  t << equal<assert, bool>(store.valid(1001, "sess1"), true) << FAS_FL;
  t << equal<assert, bool>(store.valid(1001, "sess2"), false) << FAS_FL;
  t << equal<assert, bool>(store.valid(1002, "sess1"), false) << FAS_FL;
  t << equal<assert, size_t>(store.size(), 1) << FAS_FL;

  store.revoke(1001, "sess1");
  t << equal<assert, bool>(store.valid(1001, "sess1"), false) << FAS_FL;
  t << equal<assert, size_t>(store.size(), 0) << FAS_FL;
}

UNIT(session_store2, "")
{
  using namespace fas::testing;

  wfc::pubsub::session_store store;
  store.upsert(1001, "sess1", 1);
  sleep(2);
  store.expire();
  t << equal<assert, bool>(store.valid(1001, "sess1"), false) << FAS_FL;
}

UNIT(session_store3, "")
{
  using namespace fas::testing;

  wfc::pubsub::session_store store;
  store.upsert(1001, "sess1", 3600, 2);
  sleep(1);
  store.upsert(1001, "sess2", 3600, 2);
  sleep(1);
  store.upsert(1001, "sess3", 3600, 2);

  t << equal<assert, size_t>(store.size(), 2) << FAS_FL;
  t << equal<assert, bool>(store.known(1001, "sess1"), false) << FAS_FL;
  t << equal<assert, bool>(store.known(1001, "sess2"), true) << FAS_FL;
  t << equal<assert, bool>(store.known(1001, "sess3"), true) << FAS_FL;

  store.upsert(1002, "other", 3600, 2);
  t << equal<assert, size_t>(store.size(), 3) << FAS_FL;
  t << equal<assert, bool>(store.known(1002, "other"), true) << FAS_FL;
}

}

BEGIN_SUITE(session_store, "")
  ADD_UNIT(session_store1)
  ADD_UNIT(session_store2)
  ADD_UNIT(session_store3)
END_SUITE(session_store)
