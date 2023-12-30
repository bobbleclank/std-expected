#include "bc/expected.h"

#include "comp.h"
#include "obj_constexpr.h"

#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

using namespace bc;

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

namespace {

constexpr int value_function_const_l_ref(int x) {
  const expected<Val, Err> e(std::in_place, x);
  const Val& val = e.value();
  return val.x;
}

constexpr int value_function_non_const_l_ref(int x) {
  expected<Val, Err> e(std::in_place, x);
  Val& val = e.value();
  return val.x;
}

constexpr int value_function_const_r_ref(int x) {
  const expected<Val, Err> e(std::in_place, x);
  const Val&& val = std::move(e).value();
  return val.x;
}

constexpr int value_function_non_const_r_ref(int x) {
  expected<Val, Err> e(std::in_place, x);
  Val&& val = std::move(e).value();
  return val.x;
}

} // namespace

TEST(expected_constexpr, value) {
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

template <class Tag>
constexpr int value_or_function_const_l_ref(int x) {
  const expected<Val, Err> e(Tag(), x);
  Val v(x + x);
  Val val = e.value_or(std::move(v));
  return val.x;
}

template <class Tag>
constexpr int value_or_function_non_const_r_ref(int x) {
  expected<Val, Err> e(Tag(), x);
  Val v(x + x);
  Val val = std::move(e).value_or(std::move(v));
  return val.x;
}

} // namespace

TEST(expected_constexpr, value_or) {
  {
    constexpr int x = value_or_function_const_l_ref<std::in_place_t>(1);
    ASSERT_EQ(x, 1);
  }
  {
    constexpr int x = value_or_function_const_l_ref<unexpect_t>(2);
    ASSERT_EQ(x, 2 + 2 + 101);
  }
  {
    constexpr int x = value_or_function_non_const_r_ref<std::in_place_t>(3);
    ASSERT_EQ(x, 3 + 101);
  }
  {
    constexpr int x = value_or_function_non_const_r_ref<unexpect_t>(4);
    ASSERT_EQ(x, 4 + 4 + 101);
  }
}

namespace {

constexpr bool has_value_function() {
  expected<Val, Err> e(std::in_place);
  bool b = e.has_value();
  return b;
}

constexpr bool operator_bool() {
  expected<Val, Err> e(std::in_place);
  bool b = static_cast<bool>(e);
  return b;
}

} // namespace

TEST(expected_constexpr, has_value) {
  {
    constexpr bool b = has_value_function();
    ASSERT_TRUE(b);
  }
  {
    constexpr bool b = operator_bool();
    ASSERT_TRUE(b);
  }
}

namespace {

constexpr int default_constructor() {
  expected<Val, Err> e;
  return e->x;
}

} // namespace

TEST(expected_constexpr, default_constructor) {
  {
    constexpr int x = default_constructor();
    ASSERT_EQ(x, 20100);
  }
}

namespace {

template <class Tag>
constexpr int copy_constructor(int x) {
  expected<Val, Err> other(Tag(), x);
  expected<Val, Err> e(other);
  return std::is_same_v<Tag, std::in_place_t> ? e->x : e.error().x;
}

} // namespace

TEST(expected_constexpr, copy_constructor) {
  static_assert(std::is_trivially_copy_constructible_v<Val> &&
                std::is_trivially_copy_constructible_v<Err>);
  {
    constexpr int x = copy_constructor<std::in_place_t>(1);
    ASSERT_EQ(x, 1);
  }
  {
    constexpr int x = copy_constructor<unexpect_t>(2);
    ASSERT_EQ(x, 2);
  }
}

namespace {

template <class Tag>
constexpr int move_constructor(int x) {
  expected<Val_trivial, Err_trivial> other(Tag(), x);
  expected<Val_trivial, Err_trivial> e(std::move(other));
  return std::is_same_v<Tag, std::in_place_t> ? e->x : e.error().x;
}

} // namespace

TEST(expected_constexpr, move_constructor) {
  static_assert(!std::is_trivially_move_constructible_v<Val> &&
                !std::is_trivially_move_constructible_v<Err>);
  static_assert(std::is_trivially_move_constructible_v<Val_trivial> &&
                std::is_trivially_move_constructible_v<Err_trivial>);
  {
    constexpr int x = move_constructor<std::in_place_t>(1);
    ASSERT_EQ(x, 1);
  }
  {
    constexpr int x = move_constructor<unexpect_t>(2);
    ASSERT_EQ(x, 2);
  }
}

namespace {

template <class Tag>
constexpr int explicit_copy_expected_constructor(int x) {
  expected<Arg, Arg> other(Tag(), x);
  expected<Val, Err> e(other);
  return std::is_same_v<Tag, std::in_place_t> ? e->x : e.error().x;
}

template <class Tag>
constexpr int implicit_copy_expected_constructor(int x) {
  expected<Arg, Arg> other(Tag(), x);
  expected<Val_implicit, Err_implicit> e = other;
  return std::is_same_v<Tag, std::in_place_t> ? e->x : e.error().x;
}

} // namespace

TEST(expected_constexpr, copy_expected_constructor) {
  {
    constexpr int x = explicit_copy_expected_constructor<std::in_place_t>(1);
    ASSERT_EQ(x, 1);
  }
  {
    constexpr int x = explicit_copy_expected_constructor<unexpect_t>(2);
    ASSERT_EQ(x, 2);
  }
  {
    constexpr int x = implicit_copy_expected_constructor<std::in_place_t>(3);
    ASSERT_EQ(x, 3);
  }
  {
    constexpr int x = implicit_copy_expected_constructor<unexpect_t>(4);
    ASSERT_EQ(x, 4);
  }
}

namespace {

template <class Tag>
constexpr int explicit_move_expected_constructor(int x) {
  expected<Arg, Arg> other(Tag(), x);
  expected<Val, Err> e(std::move(other));
  return std::is_same_v<Tag, std::in_place_t> ? e->x : e.error().x;
}

template <class Tag>
constexpr int implicit_move_expected_constructor(int x) {
  expected<Arg, Arg> other(Tag(), x);
  expected<Val_implicit, Err_implicit> e = std::move(other);
  return std::is_same_v<Tag, std::in_place_t> ? e->x : e.error().x;
}

} // namespace

TEST(expected_constexpr, move_expected_constructor) {
  {
    constexpr int x = explicit_move_expected_constructor<std::in_place_t>(1);
    ASSERT_EQ(x, 1 + 201);
  }
  {
    constexpr int x = explicit_move_expected_constructor<unexpect_t>(2);
    ASSERT_EQ(x, 2 + 201);
  }
  {
    constexpr int x = implicit_move_expected_constructor<std::in_place_t>(3);
    ASSERT_EQ(x, 3 + 201);
  }
  {
    constexpr int x = implicit_move_expected_constructor<unexpect_t>(4);
    ASSERT_EQ(x, 4 + 201);
  }
}

namespace {

template <class U>
constexpr int explicit_value_constructor(int x) {
  U val(x);
  expected<Val_explicit, Err> e(std::move(val));
  return e->x;
}

template <class U>
constexpr int implicit_value_constructor(int x) {
  U val(x);
  expected<Val_implicit, Err> e = std::move(val);
  return e->x;
}

} // namespace

TEST(expected_constexpr, value_constructor) {
  {
    constexpr int x = explicit_value_constructor<Val_explicit>(1);
    ASSERT_EQ(x, 1 + 401);
  }
  {
    constexpr int x = explicit_value_constructor<Arg>(2);
    ASSERT_EQ(x, 2 + 201);
  }
  {
    constexpr int x = implicit_value_constructor<Val_implicit>(3);
    ASSERT_EQ(x, 3 + 301);
  }
  {
    constexpr int x = implicit_value_constructor<Arg>(4);
    ASSERT_EQ(x, 4 + 201);
  }
}

namespace {

template <class U>
constexpr int explicit_copy_unexpected_constructor(int x) {
  unexpected<U> val(x);
  expected<Val, Err_explicit> e(val);
  return e.error().x;
}

template <class U>
constexpr int implicit_copy_unexpected_constructor(int x) {
  unexpected<U> val(x);
  expected<Val, Err_implicit> e = val;
  return e.error().x;
}

} // namespace

TEST(expected_constexpr, copy_unexpected_constructor) {
  {
    constexpr int x = explicit_copy_unexpected_constructor<Err_explicit>(1);
    ASSERT_EQ(x, 1);
  }
  {
    constexpr int x = explicit_copy_unexpected_constructor<Arg>(2);
    ASSERT_EQ(x, 2);
  }
  {
    constexpr int x = implicit_copy_unexpected_constructor<Err_implicit>(3);
    ASSERT_EQ(x, 3);
  }
  {
    constexpr int x = implicit_copy_unexpected_constructor<Arg>(4);
    ASSERT_EQ(x, 4);
  }
}

namespace {

template <class U>
constexpr int explicit_move_unexpected_constructor(int x) {
  unexpected<U> val(x);
  expected<Val, Err_explicit> e(std::move(val));
  return e.error().x;
}

template <class U>
constexpr int implicit_move_unexpected_constructor(int x) {
  unexpected<U> val(x);
  expected<Val, Err_implicit> e = std::move(val);
  return e.error().x;
}

} // namespace

TEST(expected_constexpr, move_unexpected_constructor) {
  {
    constexpr int x = explicit_move_unexpected_constructor<Err_explicit>(1);
    ASSERT_EQ(x, 1 + 401);
  }
  {
    constexpr int x = explicit_move_unexpected_constructor<Arg>(2);
    ASSERT_EQ(x, 2 + 201);
  }
  {
    constexpr int x = implicit_move_unexpected_constructor<Err_implicit>(3);
    ASSERT_EQ(x, 3 + 301);
  }
  {
    constexpr int x = implicit_move_unexpected_constructor<Arg>(4);
    ASSERT_EQ(x, 4 + 201);
  }
}

namespace {

template <class Tag, class... Args>
constexpr int in_place_constructor(Args&&... args) {
  expected<Val, Err> e(Tag(), std::forward<Args>(args)...);
  return std::is_same_v<Tag, std::in_place_t> ? e->x : e.error().x;
}

template <class Tag, class... Args>
constexpr int in_place_constructor(std::initializer_list<int> il,
                                   Args&&... args) {
  expected<Val, Err> e(Tag(), il, std::forward<Args>(args)...);
  return std::is_same_v<Tag, std::in_place_t> ? e->x : e.error().x;
}

} // namespace

TEST(expected_constexpr, in_place_constructor) {
  {
    constexpr int x = in_place_constructor<std::in_place_t>();
    ASSERT_EQ(x, 20100);
  }
  {
    constexpr int x = in_place_constructor<std::in_place_t>(Arg(1), 1);
    ASSERT_EQ(x, 1 + 1 + 201);
  }
  {
    constexpr int x = in_place_constructor<unexpect_t>();
    ASSERT_EQ(x, 20100);
  }
  {
    constexpr int x = in_place_constructor<unexpect_t>(Arg(2), 2);
    ASSERT_EQ(x, 2 + 2 + 201);
  }
  {
    constexpr int x = in_place_constructor<std::in_place_t>({3}, Arg(3), 3);
    ASSERT_EQ(x, 3 + 3 + 3 + 201);
  }
  {
    constexpr int x = in_place_constructor<unexpect_t>({4}, Arg(4), 4);
    ASSERT_EQ(x, 4 + 4 + 4 + 201);
  }
}

namespace {

template <class Compare>
constexpr bool equality_operators(int x, int y) {
  expected<Val, Err> e1(std::in_place, x);
  expected<Val, Err> e2(std::in_place, y);
  return Compare()(e1, e2);
}

} // namespace

TEST(expected_constexpr, equality_operators) {
  {
    constexpr bool b = equality_operators<Equal_to>(1, 1);
    ASSERT_TRUE(b);
  }
  {
    constexpr bool b = equality_operators<Not_equal_to>(1, 2);
    ASSERT_TRUE(b);
  }
}

namespace {

template <class Compare>
constexpr bool comparison_with_T_left(int x, int y) {
  Val v(x);
  expected<Val, Err> e(std::in_place, y);
  return Compare()(v, e);
}

template <class Compare>
constexpr bool comparison_with_T_right(int x, int y) {
  expected<Val, Err> e(std::in_place, x);
  Val v(y);
  return Compare()(e, v);
}

} // namespace

TEST(expected_constexpr, comparison_with_T) {
  {
    constexpr bool b = comparison_with_T_left<Equal_to>(1, 1);
    ASSERT_TRUE(b);
  }
  {
    constexpr bool b = comparison_with_T_right<Equal_to>(2, 2);
    ASSERT_TRUE(b);
  }
  {
    constexpr bool b = comparison_with_T_left<Not_equal_to>(1, 2);
    ASSERT_TRUE(b);
  }
  {
    constexpr bool b = comparison_with_T_right<Not_equal_to>(2, 1);
    ASSERT_TRUE(b);
  }
}

namespace {

template <class Compare>
constexpr bool comparison_with_unexpected_E_left(int x, int y) {
  unexpected<Err> v(x);
  expected<Val, Err> e(unexpect, y);
  return Compare()(v, e);
}

template <class Compare>
constexpr bool comparison_with_unexpected_E_right(int x, int y) {
  expected<Val, Err> e(unexpect, x);
  unexpected<Err> v(y);
  return Compare()(e, v);
}

} // namespace

TEST(expected_constexpr, comparison_with_unexpected_E) {
  {
    constexpr bool b = comparison_with_unexpected_E_left<Equal_to>(1, 1);
    ASSERT_TRUE(b);
  }
  {
    constexpr bool b = comparison_with_unexpected_E_right<Equal_to>(2, 2);
    ASSERT_TRUE(b);
  }
  {
    constexpr bool b = comparison_with_unexpected_E_left<Not_equal_to>(1, 2);
    ASSERT_TRUE(b);
  }
  {
    constexpr bool b = comparison_with_unexpected_E_right<Not_equal_to>(2, 1);
    ASSERT_TRUE(b);
  }
}
