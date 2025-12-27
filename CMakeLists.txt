cmake_minimum_required(VERSION 3.23)

project(bc_std_expected VERSION 1.2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra -pedantic -Werror)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_library(bcexpected INTERFACE)
target_sources(bcexpected
  INTERFACE
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/bc/expected.h
)

add_subdirectory(test)

install(
  TARGETS
    bcexpected
  FILE_SET HEADERS
)
