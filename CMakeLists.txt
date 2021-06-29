cmake_minimum_required(VERSION 3.10)
project(APExtInt
  VERSION 0.1.0
  DESCRIPTION "A ap_int alternative using _ExtInt as backend"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to Release as none was specified.")
  set(CMAKE_BUILD_TYPE "Release" CACHE
    STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(INCLUDE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(APExtInt INTERFACE)
target_include_directories(APExtInt
  INTERFACE
    $<BUILD_INTERFACE:${INCLUDE_ROOT}>
    $<INSTALL_INTERFACE:include>
)

install(
  DIRECTORY ${INCLUDE_ROOT}/ 
  DESTINATION include/ap_ext_int 
  FILES_MATCHING PATTERN "*.hpp"
)
install(
  TARGETS APExtInt  EXPORT APExtIntTargets
  INCLUDES DESTINATION include
) 
install(
  EXPORT APExtIntTargets
  FILE APExtIntTargets.cmake
  NAMESPACE APExtInt::
  DESTINATION lib/cmake/APExtInt
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  option(BUILD_TESTING "Build the test tree")
  if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
  endif()
endif()
