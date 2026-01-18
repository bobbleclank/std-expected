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
include(CMakePackageConfigHelpers)
install(
  TARGETS
    bcexpected
  EXPORT BcExpectedTargets
  FILE_SET HEADERS
)
add_library(BcExpected::bcexpected ALIAS bcexpected)
install(
  EXPORT BcExpectedTargets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/BcExpected
  NAMESPACE BcExpected::
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/BcExpectedConfigVersion.cmake
  COMPATIBILITY ExactVersion
)
install(
  FILES
    cmake/BcExpectedConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/BcExpectedConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/BcExpected
)
