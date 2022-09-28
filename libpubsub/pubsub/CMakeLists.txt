add_library(
  pubsub
    message_queue.cpp
    channel_map.cpp
    pubsub.cpp
    pubsub_mt.cpp
    rocksdb/rocksdb.cpp
    rocksdb/multi_rocksdb.cpp
    rocksdb/rocksdb_factory.cpp
    rocksdb/merge_operator.cpp
)

wci_targets(pubsub)
get_boost(filesystem)

add_dependencies(pubsub rocksdb )
target_include_directories(pubsub PUBLIC ${libpubsub_SOURCE_DIR})
target_link_libraries(pubsub PUBLIC rocksdb PRIVATE wlog faslib ${SNAPPY_LIBRARIES} Boost::filesystem)
