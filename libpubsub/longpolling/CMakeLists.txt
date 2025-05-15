add_library(
  longpolling
    agent.cpp
    agent_map.cpp
    agent_map_mt.cpp
    subscribe_manager.cpp
    longpolling.cpp
)

update_ogenc(
  TARGETS longpolling
  WARNINGS -Wcast-align -Wundef -Wcast-align -Wsign-conversion -Warith-conversion -Wold-style-cast
           -Wctor-dtor-privacy -Wuseless-cast -Wcast-align
  OFF)

wci_targets(longpolling)

target_compile_options(longpolling PRIVATE -Wno-cast-align -Wno-sign-conversion)

get_boost(atomic)

add_dependencies(pubsub message_queue)
target_include_directories(longpolling PUBLIC "$<BUILD_INTERFACE:${libpubsub_SOURCE_DIR}>" PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(longpolling PRIVATE wlog faslib pubsub ${Boost_LIBRARIES})
