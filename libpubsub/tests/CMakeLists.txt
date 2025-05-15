add_executable(
  test_pubsub
    test_pubsub.cpp
    agent_suite.cpp
    message_suite.cpp
    longpolling_suite.cpp
)

target_link_libraries(test_pubsub PUBLIC pubsub longpolling message_queue fas::testing wfc)

wci_test(TARGET test_pubsub COMMAND test_pubsub)
