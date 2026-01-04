cmake_minimum_required(VERSION 3.23)

project(bc_std_expected VERSION 1.2)

option(BCEXPECTED_BUILD_TESTS "Build the unit tests" ON)

add_subdirectory(include)

if(BCEXPECTED_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

install(
  TARGETS
    bcexpected
  FILE_SET HEADERS
)
