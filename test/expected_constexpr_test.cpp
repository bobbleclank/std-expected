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
