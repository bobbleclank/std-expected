cmake_minimum_required(VERSION 3.23)

project(bc_expected
  VERSION 1.2
)

option(BCEXPECTED_BUILD_TESTS "Build the unit tests" ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
  FIND_PACKAGE_ARGS NAMES GTest CONFIG
)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

add_subdirectory(include)

if(BCEXPECTED_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

include(GNUInstallDirs)
install(
  TARGETS
    bcexpected
  EXPORT BcExpectedTargets
  FILE_SET HEADERS
)
install(
  EXPORT BcExpectedTargets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/BcExpected
  NAMESPACE BcExpected::
)
install(
  FILES
    cmake/BcExpectedConfig.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/BcExpected
)
