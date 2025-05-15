add_library(
  pubsub
    pubsub.cpp
    pubsub_mt.cpp
    rocksdb/rocksdb.cpp
    rocksdb/multi_rocksdb.cpp
    rocksdb/rocksdb_factory.cpp
    rocksdb/merge_operator.cpp
)

wci_targets(pubsub)
get_boost(filesystem atomic)

add_dependencies(pubsub rocksdb message_queue)
target_include_directories(pubsub PUBLIC "$<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>")
target_include_directories(pubsub PUBLIC "$<BUILD_INTERFACE:${libpubsub_SOURCE_DIR}>")
target_link_libraries(pubsub PUBLIC ${Boost_LIBRARIES} rocksdb message_queue PRIVATE wlog faslib  ${SNAPPY_LIBRARIES} )
