cmake_minimum_required(VERSION 3.10)

project(libpubsub CXX)

add_subdirectory(message_queue)
add_subdirectory(pubsub)
add_subdirectory(longpolling)

if (BUILD_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()

