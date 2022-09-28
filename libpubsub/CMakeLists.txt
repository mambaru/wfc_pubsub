cmake_minimum_required(VERSION 3.1)

project(libpubsub CXX)

add_subdirectory(pubsub)

if (BUILD_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()

