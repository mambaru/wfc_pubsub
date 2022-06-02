cmake_minimum_required(VERSION 3.1)

project(wfc_pubsub)

include(cmake/ci.cmake)
wci_submodule(NAME wfc SUPERMODULE)

add_subdirectory(package)

if ( BUILD_TESTING )
  enable_testing()
endif()
