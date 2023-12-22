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

namespace {

template <class Tag, class... Args>
constexpr auto copy_constructor(Args&&... args)
    -> std::conditional_t<std::is_same_v<Tag, std::in_place_t>, bool, int> {
  expected<void, Err> other(Tag(), std::forward<Args>(args)...);
  expected<void, Err> e(other);
  if constexpr (std::is_same_v<Tag, std::in_place_t>)
    return e.has_value();
  else
    return e.error().x;
}

} // namespace

TEST(expected_void_constexpr, copy_constructor) {
  static_assert(std::is_trivially_copy_constructible_v<Err>);
  {
    constexpr auto b = copy_constructor<std::in_place_t>();
    static_assert(std::is_same_v<decltype(b), const bool>);
    ASSERT_TRUE(b);
  }
  {
    constexpr auto x = copy_constructor<unexpect_t>(1);
    static_assert(std::is_same_v<decltype(x), const int>);
    ASSERT_EQ(x, 1);
  }
}

namespace {

template <class Tag, class... Args>
constexpr auto move_constructor(Args&&... args)
    -> std::conditional_t<std::is_same_v<Tag, std::in_place_t>, bool, int> {
  expected<void, Err_trivial> other(Tag(), std::forward<Args>(args)...);
  expected<void, Err_trivial> e(std::move(other));
  if constexpr (std::is_same_v<Tag, std::in_place_t>)
    return e.has_value();
  else
    return e.error().x;
}

} // namespace

TEST(expected_void_constexpr, move_constructor) {
  static_assert(!std::is_trivially_move_constructible_v<Err>);
  static_assert(std::is_trivially_move_constructible_v<Err_trivial>);
  {
    constexpr auto b = move_constructor<std::in_place_t>();
    static_assert(std::is_same_v<decltype(b), const bool>);
    ASSERT_TRUE(b);
  }
  {
    constexpr auto x = move_constructor<unexpect_t>(1);
    static_assert(std::is_same_v<decltype(x), const int>);
    ASSERT_EQ(x, 1);
  }
}

namespace {

template <class Tag, class... Args>
constexpr auto explicit_copy_expected_constructor(Args&&... args)
    -> std::conditional_t<std::is_same_v<Tag, std::in_place_t>, bool, int> {
  expected<void, Arg> other(Tag(), std::forward<Args>(args)...);
  expected<void, Err> e(other);
  if constexpr (std::is_same_v<Tag, std::in_place_t>)
    return e.has_value();
  else
    return e.error().x;
}

template <class Tag, class... Args>
constexpr auto implicit_copy_expected_constructor(Args&&... args)
    -> std::conditional_t<std::is_same_v<Tag, std::in_place_t>, bool, int> {
  expected<void, Arg> other(Tag(), std::forward<Args>(args)...);
  expected<void, Err_implicit> e = other;
  if constexpr (std::is_same_v<Tag, std::in_place_t>)
    return e.has_value();
  else
    return e.error().x;
}

} // namespace

TEST(expected_void_constexpr, copy_expected_constructor) {
  {
    constexpr auto b = explicit_copy_expected_constructor<std::in_place_t>();
    static_assert(std::is_same_v<decltype(b), const bool>);
    ASSERT_TRUE(b);
  }
  {
    constexpr auto x = explicit_copy_expected_constructor<unexpect_t>(1);
    static_assert(std::is_same_v<decltype(x), const int>);
    ASSERT_EQ(x, 1);
  }
  {
    constexpr auto b = implicit_copy_expected_constructor<std::in_place_t>();
    static_assert(std::is_same_v<decltype(b), const bool>);
    ASSERT_TRUE(b);
  }
  {
    constexpr auto x = implicit_copy_expected_constructor<unexpect_t>(2);
    static_assert(std::is_same_v<decltype(x), const int>);
    ASSERT_EQ(x, 2);
  }
}

namespace {

template <class Tag, class... Args>
constexpr auto explicit_move_expected_constructor(Args&&... args)
    -> std::conditional_t<std::is_same_v<Tag, std::in_place_t>, bool, int> {
  expected<void, Arg> other(Tag(), std::forward<Args>(args)...);
  expected<void, Err> e(std::move(other));
  if constexpr (std::is_same_v<Tag, std::in_place_t>)
    return e.has_value();
  else
    return e.error().x;
}

template <class Tag, class... Args>
constexpr auto implicit_move_expected_constructor(Args&&... args)
    -> std::conditional_t<std::is_same_v<Tag, std::in_place_t>, bool, int> {
  expected<void, Arg> other(Tag(), std::forward<Args>(args)...);
  expected<void, Err_implicit> e = std::move(other);
  if constexpr (std::is_same_v<Tag, std::in_place_t>)
    return e.has_value();
  else
    return e.error().x;
}

} // namespace

TEST(expected_void_constexpr, move_expected_constructor) {
  {
    constexpr auto b = explicit_move_expected_constructor<std::in_place_t>();
    static_assert(std::is_same_v<decltype(b), const bool>);
    ASSERT_TRUE(b);
  }
  {
    constexpr auto x = explicit_move_expected_constructor<unexpect_t>(1);
    static_assert(std::is_same_v<decltype(x), const int>);
    ASSERT_EQ(x, 201 + 1);
  }
  {
    constexpr auto b = implicit_move_expected_constructor<std::in_place_t>();
    static_assert(std::is_same_v<decltype(b), const bool>);
    ASSERT_TRUE(b);
  }
  {
    constexpr auto x = implicit_move_expected_constructor<unexpect_t>(2);
    static_assert(std::is_same_v<decltype(x), const int>);
    ASSERT_EQ(x, 201 + 2);
  }
}

namespace {

constexpr bool in_place_constructor() {
  expected<void, Err> e(std::in_place);
  return e.has_value();
}

} // namespace

TEST(expected_void_constexpr, in_place_constructor) {
  {
    constexpr bool b = in_place_constructor();
    ASSERT_TRUE(b);
  }
}
