add_library(
  longpolling
    agent.cpp
    agent_map.cpp
    agent_map_mt.cpp
    wait_list.cpp
    subscribe_manager.cpp
    longpolling.cpp
    hub_rpc_inflight.cpp
    auth/session_store.cpp
    auth/longpolling_auth.cpp
)

wci_targets(longpolling)

get_boost(atomic)

add_dependencies(pubsub message_queue)
target_include_directories(longpolling PUBLIC "$<BUILD_INTERFACE:${libpubsub_SOURCE_DIR}>" )
target_link_libraries(longpolling PRIVATE wlog faslib pubsub Boost::atomic)
