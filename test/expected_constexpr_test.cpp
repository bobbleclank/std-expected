#include "bc/exp/expected.h"

#include "obj_constexpr.h"

#include <utility>

#include <gtest/gtest.h>

using namespace bc::exp;

namespace {

constexpr int member_access_operator_const(int x) {
  const expected<Val, Err> e(std::in_place, x);
  const Val* ptr = e.operator->();
  return ptr->x;
}

constexpr int member_access_operator_non_const(int x) {
  expected<Val, Err> e(std::in_place, x);
  Val* ptr = e.operator->();
  return ptr->x;
}

} // namespace

TEST(expected_constexpr, member_access_operator) {
  {
    constexpr int x = member_access_operator_const(1);
    ASSERT_EQ(x, 1);
  }
  {
    constexpr int x = member_access_operator_non_const(2);
    ASSERT_EQ(x, 2);
  }
}

namespace {

constexpr int indirection_operator_const_l_ref(int x) {
  const expected<Val, Err> e(std::in_place, x);
  const Val& val = *e;
  return val.x;
}

constexpr int indirection_operator_non_const_l_ref(int x) {
  expected<Val, Err> e(std::in_place, x);
  Val& val = *e;
  return val.x;
}

constexpr int indirection_operator_const_r_ref(int x) {
  const expected<Val, Err> e(std::in_place, x);
  const Val&& val = *std::move(e);
  return val.x;
}

constexpr int indirection_operator_non_const_r_ref(int x) {
  expected<Val, Err> e(std::in_place, x);
  Val&& val = *std::move(e);
  return val.x;
}

} // namespace

TEST(expected_constexpr, indirection_operator) {
  {
    constexpr int x = indirection_operator_const_l_ref(1);
    ASSERT_EQ(x, 1);
  }
  {
    constexpr int x = indirection_operator_non_const_l_ref(2);
    ASSERT_EQ(x, 2);
  }
  {
    constexpr int x = indirection_operator_const_r_ref(3);
    ASSERT_EQ(x, 3);
  }
  {
    constexpr int x = indirection_operator_non_const_r_ref(4);
    ASSERT_EQ(x, 4);
  }
}

namespace {

constexpr int error_function_const_l_ref(int x) {
  const expected<Val, Err> e(unexpect, x);
  const Err& err = e.error();
  return err.x;
}

constexpr int error_function_non_const_l_ref(int x) {
  expected<Val, Err> e(unexpect, x);
  Err& err = e.error();
  return err.x;
}

constexpr int error_function_const_r_ref(int x) {
  const expected<Val, Err> e(unexpect, x);
  const Err&& err = std::move(e).error();
  return err.x;
}

constexpr int error_function_non_const_r_ref(int x) {
  expected<Val, Err> e(unexpect, x);
  Err&& err = std::move(e).error();
  return err.x;
}

} // namespace

TEST(expected_constexpr, error) {
  {
    constexpr int x = error_function_const_l_ref(1);
    ASSERT_EQ(x, 1);
  }
  {
    constexpr int x = error_function_non_const_l_ref(2);
    ASSERT_EQ(x, 2);
  }
  {
    constexpr int x = error_function_const_r_ref(3);
    ASSERT_EQ(x, 3);
  }
  {
    constexpr int x = error_function_non_const_r_ref(4);
    ASSERT_EQ(x, 4);
  }
}
