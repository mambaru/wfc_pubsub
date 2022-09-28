add_executable(test_pubsub test_pubsub.cpp message_suite.cpp)
target_link_libraries(test_pubsub PUBLIC pubsub fas::testing)
wci_test(TARGET test_pubsub COMMAND test_pubsub)
