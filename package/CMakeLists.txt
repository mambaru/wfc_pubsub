
add_library(wfc_pubsub
  pubsub_package.cpp
)

wci_targets(wfc_pubsub)
enable_stat(wfc_pubsub)

target_link_libraries(wfc_pubsub PUBLIC wfc )

target_include_directories(wfc_pubsub PUBLIC ${wfc_pubsub_SOURCE_DIR})
target_include_directories(wfc_pubsub PUBLIC ${wfc_pubsub_SOURCE_DIR}/package)

build_info(wfc_pubsub pubsub)
