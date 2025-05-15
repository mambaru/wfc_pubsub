
add_executable(test_package test_package.cpp package_suite.cpp longpoll_suite.cpp)

target_link_libraries(test_package PRIVATE wfc_pubsub fas::testing)

wci_test(TARGET test_package COMMAND test_package)

add_executable(where_memory1 where_memory1.cpp)
add_executable(where_memory2 where_memory2.cpp)
add_executable(where_memory3 where_memory3.cpp)

target_link_libraries(where_memory1 PRIVATE pubsub longpolling faslib wlog )
target_link_libraries(where_memory2 PRIVATE pubsub longpolling faslib wlog )
target_link_libraries(where_memory3 PRIVATE pubsub longpolling faslib wlog )

wci_utils(where_memory1 where_memory2 where_memory3)
