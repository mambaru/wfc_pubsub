add_executable(
  test_pubsub
    test_pubsub.cpp
    agent_suite.cpp
    message_suite.cpp
    longpolling_suite.cpp
    session_store_suite.cpp
    rocksdb_suite.cpp
)

get_boost(filesystem)
target_link_libraries(test_pubsub PRIVATE pubsub longpolling message_queue fas::testing wfc rocksdb Boost::filesystem ${SNAPPY_LIBRARIES})
target_include_directories(test_pubsub PRIVATE "$<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>")

wci_test(TARGET test_pubsub COMMAND test_pubsub)
