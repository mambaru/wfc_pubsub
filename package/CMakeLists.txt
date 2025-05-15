get_boost(url)

add_library(wfc_pubsub
  pubsub_package.cpp

  pubsub/pubsub_module.cpp
  pubsub/domain/pubsub_multiton.cpp
  pubsub/domain/pubsub_domain.cpp
  pubsub/service/pubsub_service_multiton.cpp
  pubsub/gateway/pubsub_gateway_multiton.cpp

  comet_adapter/comet_adapter_module.cpp
  comet_adapter/domain/comet_adapter_multiton.cpp
  comet_adapter/domain/comet_adapter_domain.cpp
  comet_adapter/service/comet_adapter_service_multiton.cpp

  longpolling/longpolling_module.cpp
  longpolling/domain/longpolling_multiton.cpp
  longpolling/domain/longpolling_domain.cpp
  longpolling/service/longpolling_service_multiton.cpp

  http/http_module.cpp
  http/domain/http_multiton.cpp
  http/domain/http_domain.cpp
)

update_ogenc(TARGETS wfc_pubsub WARNINGS -Wsign-conversion OFF)
set_source_files_properties(http/domain/http_domain.cpp PROPERTIES COMPILE_FLAGS "-Wno-null-dereference")

wci_targets(wfc_pubsub)
enable_stat(wfc_pubsub)


target_link_libraries(wfc_pubsub PUBLIC wfc pubsub longpolling ${Boost_LIBRARIES})

target_include_directories(wfc_pubsub PUBLIC "$<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>")
target_include_directories(wfc_pubsub PUBLIC "$<BUILD_INTERFACE:${wfc_pubsub_SOURCE_DIR}>")
target_include_directories(wfc_pubsub PUBLIC "$<BUILD_INTERFACE:${wfc_pubsub_SOURCE_DIR}/package>")

build_info(wfc_pubsub pubsub)

install(TARGETS pubsub longpolling message_queue EXPORT wfc_pubsubTargets)


