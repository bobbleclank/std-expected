cmake_minimum_required(VERSION 3.23)

project(bc_std_expected VERSION 1.2)

add_subdirectory(include)
add_subdirectory(test)

install(
  TARGETS
    bcexpected
  FILE_SET HEADERS
)
