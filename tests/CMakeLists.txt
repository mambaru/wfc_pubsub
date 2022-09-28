
add_executable(test_package test_package.cpp package_suite.cpp)

target_link_libraries(test_package PRIVATE wfc_pubsub fas::testing)

wci_test(TARGET test_package COMMAND test_package)


