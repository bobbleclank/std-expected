cmake_minimum_required(VERSION 3.20)

project(bc_std_expected VERSION 1.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -pedantic -Werror)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_library(bcexpected INTERFACE)
target_include_directories(bcexpected INTERFACE "${CMAKE_SOURCE_DIR}/include")

add_subdirectory(test)
