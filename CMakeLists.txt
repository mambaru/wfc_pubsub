cmake_minimum_required(VERSION 3.1)

project(wfc_pubsub)

include(cmake/ci.cmake)
include(cmake/rocksdb.cmake)

wci_submodule(NAME wfc SUPERMODULE)
get_rocksdb()

add_subdirectory(libpubsub)
add_subdirectory(package)

if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory(tests)
endif()
