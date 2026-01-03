cmake_minimum_required(VERSION 3.23)

project(bc_std_expected VERSION 1.2)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(include)
add_subdirectory(test)

install(
  TARGETS
    bcexpected
  FILE_SET HEADERS
)
