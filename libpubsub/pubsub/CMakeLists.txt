add_library(
  pubsub
    pubsub.cpp
    pubsub_mt.cpp
    rocksdb/rocksdb.cpp
    rocksdb/multi_rocksdb.cpp
    rocksdb/rocksdb_factory.cpp
    rocksdb/merge_operator.cpp
)

update_ogenc(
  TARGETS pubsub
  WARNINGS -Wuseless-cast -Wctor-dtor-privacy -Wzero-as-null-pointer-constant -Wold-style-cast -Wnrvo -Wcast-align -Wredundant-decls -Warith-conversion
  OFF)

update_ogenc(
  TARGETS pubsub
  WARNINGS -Wno-cast-align
  ON)

wci_targets(pubsub)
get_boost(filesystem atomic)

add_dependencies(pubsub rocksdb message_queue)
target_include_directories(pubsub PUBLIC "$<BUILD_INTERFACE:${libpubsub_SOURCE_DIR}>")
target_link_libraries(pubsub PRIVATE Boost::filesystem Boost::atomic rocksdb message_queue PRIVATE wlog faslib  ${SNAPPY_LIBRARIES} )
