add_executable(test_std_expected
  bad_expected_access_test.cpp
  copy_base_test.cpp
  expected_test.cpp
  move_base_test.cpp
  operations_base_test.cpp
  storage_base_test.cpp
  unexpected_test.cpp)
target_link_libraries(test_std_expected PUBLIC expected gtest_main gtest)
