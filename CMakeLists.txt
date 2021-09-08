cmake_minimum_required(VERSION 3.20)

project(std_expected VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -pedantic -Werror)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_library(expected INTERFACE)
target_include_directories(expected INTERFACE "${CMAKE_SOURCE_DIR}/include")

add_subdirectory(test)
