
add_executable(avl_map_test avl_map_test.cpp)

target_link_libraries(avl_map_test gtest_main)

enable_testing()
add_test(NAME avl_map_test COMMAND test)