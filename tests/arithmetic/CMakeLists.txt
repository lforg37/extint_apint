add_executable(arithmetic arithmetic.cpp conversion.cpp)
target_link_libraries(arithmetic PRIVATE APExtInt Boost::unit_test_framework)
add_test(NAME arithmetic COMMAND arithmetic)
