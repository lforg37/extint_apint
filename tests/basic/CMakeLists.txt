add_executable(simple_compile simple_compile.cpp)
target_link_libraries(simple_compile PRIVATE APExtInt)
add_test(NAME simple_compile COMMAND simple_compile)
