#include <fas/testing.hpp>

BEGIN_TEST
  RUN_SUITE(message_suite)
  RUN_SUITE(agent_suite)
  RUN_SUITE(longpolling_suite)
  RUN_SUITE(session_store)
  RUN_SUITE(rocksdb_suite)
END_TEST
