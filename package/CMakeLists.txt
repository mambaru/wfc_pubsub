
add_library(wfc_pubsub
  pubsub_package.cpp

  pubsub/pubsub_module.cpp
  pubsub/domain/pubsub_multiton.cpp
  pubsub/domain/pubsub_domain.cpp
  pubsub/service//pubsub_service_multiton.cpp

  comet_adapter/comet_adapter_module.cpp
  comet_adapter/domain/comet_adapter_multiton.cpp
  comet_adapter/domain/comet_adapter_domain.cpp
  comet_adapter/service/comet_adapter_service_multiton.cpp
)

wci_targets(wfc_pubsub)
enable_stat(wfc_pubsub)

target_link_libraries(wfc_pubsub PUBLIC wfc pubsub)

target_include_directories(wfc_pubsub PUBLIC ${wfc_pubsub_SOURCE_DIR})
target_include_directories(wfc_pubsub PUBLIC ${wfc_pubsub_SOURCE_DIR}/package)

build_info(wfc_pubsub pubsub)
