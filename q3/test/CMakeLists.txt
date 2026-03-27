add_executable(udp_class_tests udp_class_tests.cpp)
target_include_directories(udp_class_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(udp_class_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../doctest/doctest)
target_link_libraries(udp_class_tests PRIVATE udp_class)

add_test(NAME UdpTests COMMAND udp_class_tests)
