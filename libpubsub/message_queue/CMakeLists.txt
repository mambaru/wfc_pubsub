add_library(
  message_queue
    message_queue.cpp
    channel_map.cpp
)

wci_targets(message_queue)
target_include_directories(message_queue PUBLIC "$<BUILD_INTERFACE:${libpubsub_SOURCE_DIR}>")
target_link_libraries(message_queue PRIVATE wlog faslib)
