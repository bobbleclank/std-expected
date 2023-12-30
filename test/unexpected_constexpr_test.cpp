#include "bc/expected.h"

#include "comp.h"
#include "obj_constexpr.h"

#include <utility>

#include <gtest/gtest.h>

using namespace bc;

namespace {

constexpr int value_function_const_l_ref(int x) {
  const unexpected<Err> e(x);
  const Err& err = e.value();
  return err.x;
}

constexpr int value_function_non_const_l_ref(int x) {
  unexpected<Err> e(x);
  Err& err = e.value();
  return err.x;
}

constexpr int value_function_const_r_ref(int x) {
  const unexpected<Err> e(x);
  const Err&& err = std::move(e).value();
  return err.x;
}

constexpr int value_function_non_const_r_ref(int x) {
  unexpected<Err> e(x);
  Err&& err = std::move(e).value();
  return err.x;
}

} // namespace

TEST(unexpected_constexpr, value) {
  {
    constexpr int x = value_function_const_l_ref(1);
    ASSERT_EQ(x, 1);
  }
  {
    constexpr int x = value_function_non_const_l_ref(2);
    ASSERT_EQ(x, 2);
  }
  {
    constexpr int x = value_function_const_r_ref(3);
    ASSERT_EQ(x, 3);
  }
  {
    constexpr int x = value_function_non_const_r_ref(4);
    ASSERT_EQ(x, 4);
  }
}

namespace {

constexpr int deduction_guide_copy(int x) {
  Err val(x);
  unexpected e(val);
  return e.value().x;
}

constexpr int deduction_guide_move(int x) {
  Err val(x);
  unexpected e(std::move(val));
  return e.value().x;
}

constexpr int copy_constructor(int x) {
  unexpected<Err> other(x);
  unexpected<Err> e(other);
  return e.value().x;
}

constexpr int move_constructor(int x) {
  unexpected<Err> other(x);
  unexpected<Err> e(std::move(other));
  return e.value().x;
}

constexpr int explicit_copy_unexpected_constructor(int x) {
  unexpected<Arg> other(x);
  unexpected<Err> e(other);
  return e.value().x;
}

constexpr int implicit_copy_unexpected_constructor(int x) {
  unexpected<Arg> other(x);
  unexpected<Err_implicit> e = other;
  return e.value().x;
}

constexpr int explicit_move_unexpected_constructor(int x) {
  unexpected<Arg> other(x);
  unexpected<Err> e(std::move(other));
  return e.value().x;
}

constexpr int implicit_move_unexpected_constructor(int x) {
  unexpected<Arg> other(x);
  unexpected<Err_implicit> e = std::move(other);
  return e.value().x;
}

template <class E>
constexpr int value_constructor(int x) {
  E val(x);
  unexpected<Err> e(std::move(val));
  return e.value().x;
}

constexpr int in_place_constructor(int x) {
  Arg arg(x);
  unexpected<Err> e(std::in_place, std::move(arg), x);
  return e.value().x;
}

constexpr int in_place_initializer_list_constructor(int x) {
  Arg arg(x);
  unexpected<Err> e(std::in_place, {x}, std::move(arg), x);
  return e.value().x;
}

} // namespace

TEST(unexpected_constexpr, constructors) {
  {
    constexpr int x = deduction_guide_copy(11);
    ASSERT_EQ(x, 11);
  }
  {
    constexpr int x = deduction_guide_move(12);
    ASSERT_EQ(x, 12 + 101);
  }
  {
    constexpr int x = copy_constructor(1);
    ASSERT_EQ(x, 1);
  }
  {
    constexpr int x = move_constructor(2);
    ASSERT_EQ(x, 2 + 101);
  }
  {
    constexpr int x = explicit_copy_unexpected_constructor(3);
    ASSERT_EQ(x, 3);
  }
  {
    constexpr int x = implicit_copy_unexpected_constructor(4);
    ASSERT_EQ(x, 4);
  }
  {
    constexpr int x = explicit_move_unexpected_constructor(5);
    ASSERT_EQ(x, 5 + 201);
  }
  {
    constexpr int x = implicit_move_unexpected_constructor(6);
    ASSERT_EQ(x, 6 + 201);
  }
  {
    constexpr int x = value_constructor<Err>(7);
    ASSERT_EQ(x, 7 + 101);
  }
  {
    constexpr int x = value_constructor<Arg>(8);
    ASSERT_EQ(x, 8 + 201);
  }
  {
    constexpr int x = in_place_constructor(9);
    ASSERT_EQ(x, 9 + 9 + 201);
  }
  {
    constexpr int x = in_place_initializer_list_constructor(10);
    ASSERT_EQ(x, 10 + 10 + 10 + 201);
  }
}

namespace {

constexpr int copy_assignment(int x) {
  unexpected<Err> other(x);
  unexpected<Err> e(10 * x);
  e = other;
  return e.value().x;
}

constexpr int move_assignment(int x) {
  unexpected<Err> other(x);
  unexpected<Err> e(10 * x);
  e = std::move(other);
  return e.value().x;
}

constexpr int copy_unexpected_assignment(int x) {
  unexpected<Arg> other(x);
  unexpected<Err> e(10 * x);
  e = other;
  return e.value().x;
}

constexpr int move_unexpected_assignment(int x) {
  unexpected<Arg> other(x);
  unexpected<Err> e(10 * x);
  e = std::move(other);
  return e.value().x;
}

} // namespace

TEST(unexpected_constexpr, assignment_operators) {
  {
    constexpr int x = copy_assignment(1);
    ASSERT_EQ(x, 1);
  }
  {
    constexpr int x = move_assignment(2);
    ASSERT_EQ(x, 2 + 102);
  }
  {
    constexpr int x = copy_unexpected_assignment(3);
    ASSERT_EQ(x, 3);
  }
  {
    constexpr int x = move_unexpected_assignment(4);
    ASSERT_EQ(x, 4 + 202);
  }
}

namespace {

template <class Compare>
constexpr bool equality_operators(int x, int y) {
  unexpected<Err> e1(x);
  unexpected<Err> e2(y);
  return Compare()(e1, e2);
}

} // namespace

TEST(unexpected_constexpr, equality_operators) {
  {
    constexpr bool b = equality_operators<Equal_to>(1, 1);
    ASSERT_TRUE(b);
  }
  {
    constexpr bool b = equality_operators<Not_equal_to>(1, 2);
    ASSERT_TRUE(b);
  }
}
