FetchContent_MakeAvailable(googletest)

add_executable(test_bcexpected)
target_sources(test_bcexpected
  PRIVATE
    bad_expected_access_test.cpp
    copy_assign_base_test.cpp
    copy_base_test.cpp
    expected_constexpr_test.cpp
    expected_test.cpp
    expected_void_constexpr_test.cpp
    expected_void_test.cpp
    move_assign_base_test.cpp
    move_base_test.cpp
    operations_base_test.cpp
    storage_base_test.cpp
    unexpected_constexpr_test.cpp
    unexpected_test.cpp
)
target_link_libraries(test_bcexpected
  PRIVATE
    bcexpected
    GTest::gtest_main
    GTest::gtest
)
target_compile_features(test_bcexpected
  PRIVATE
    cxx_std_23
)
target_compile_options(test_bcexpected
  PRIVATE
    -Wall
    -Wextra
    -pedantic
    -Werror
)

add_test(
  NAME test_bcexpected
  COMMAND test_bcexpected
)
