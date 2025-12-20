add_executable(test_${PROJECT_NAME})
target_sources(test_${PROJECT_NAME}
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
target_link_libraries(test_${PROJECT_NAME} PRIVATE bcexpected gtest_main gtest)
