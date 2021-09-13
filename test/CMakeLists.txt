add_executable(test_std_expected
  bad_expected_access_test.cpp
  expected_test.cpp
  unexpected_test.cpp)
target_link_libraries(test_std_expected PUBLIC expected gtest_main gtest)
