#include "bc/exp/expected.h"

#include "obj_constexpr.h"

#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

using namespace bc::exp;

namespace {

constexpr bool value_function() {
  expected<void, Err> e(std::in_place);
  static_assert(std::is_same_v<decltype(e.value()), void>);
  return e.has_value();
}

} // namespace

TEST(expected_void_constexpr, value) {
  {
    constexpr bool b = value_function();
    ASSERT_TRUE(b);
  }
}

namespace {

constexpr bool default_constructor() {
  expected<void, Err> e;
  return e.has_value();
}

} // namespace

TEST(expected_void_constexpr, default_constructor) {
  {
    constexpr bool b = default_constructor();
    ASSERT_TRUE(b);
  }
}
