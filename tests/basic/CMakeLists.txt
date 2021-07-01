add_executable(simple_compile simple_compile.cpp)
target_link_libraries(simple_compile PRIVATE APExtInt)
add_test(NAME simple_compile COMMAND simple_compile)

add_executable(arithmetic arithmetic.cpp)
target_link_libraries(arithmetic PRIVATE APExtInt Boost::unit_test_framework)
add_test(NAME arithmetic COMMAND arithmetic)
