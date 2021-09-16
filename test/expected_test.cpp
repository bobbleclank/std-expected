#include "exp/expected.h"

#include <initializer_list>
#include <utility>

#include <gtest/gtest.h>

using namespace exp;

namespace {

enum class State {
  none,
  default_constructed,
  single_parameter_constructed,
  args_constructed,
  list_constructed,
  copy_constructed,
  move_constructed,
  copy_assigned,
  move_assigned,
  destructed
};

struct Arg {
  Arg() = default;
  explicit Arg(int x_) : x(x_) {}

  int x = 0;
};

template <class Tag> struct Obj {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  Obj() { s = State::default_constructed; }

  explicit Obj(int x_) {
    s = State::single_parameter_constructed;
    x = x_;
  }

  Obj(const Arg& arg_, int) {
    s = State::args_constructed;
    Arg arg = arg_;
    x = arg.x;
  }

  Obj(std::initializer_list<int>, const Arg& arg_, int) {
    s = State::list_constructed;
    Arg arg = arg_;
    x = arg.x;
  }

  Obj(const Obj& other) {
    s = State::copy_constructed;
    x = other.x;
  }

  Obj(Obj&& other) {
    s = State::move_constructed;
    x = other.x;
    other.x = -1;
  }

  Obj& operator=(const Obj& other) {
    s = State::copy_assigned;
    x = other.x;
    return *this;
  }

  Obj& operator=(Obj&& other) {
    s = State::move_assigned;
    x = other.x;
    other.x = -2;
    return *this;
  }

  ~Obj() { s = State::destructed; }

  int x = 0;
};

struct Val_tag {};
using Val = Obj<Val_tag>;

struct Err_tag {};
using Err = Obj<Err_tag>;

} // namespace

TEST(expected, member_access_operator) {
  // const overload
  {
    const expected<Val, Err> e(std::in_place, 1);
    ASSERT_EQ(e.operator->(), &*e);
    ASSERT_EQ(e->x, 1);
    ASSERT_EQ((*e).x, 1);
  }
  // non-const overload
  {
    expected<Val, Err> e(std::in_place, 2);
    ASSERT_EQ(e.operator->(), &*e);
    ASSERT_EQ(e->x, 2);
    ASSERT_EQ((*e).x, 2);
    e->x = 20;
    ASSERT_EQ(e->x, 20);
    ASSERT_EQ((*e).x, 20);
  }
}

TEST(expected, indirection_operator) {
  // const& overload
  {
    const expected<Val, Err> e(std::in_place, 1);
    const Val& val = *e;
    ASSERT_EQ(val.x, 1);
    ASSERT_EQ((*e).x, 1);
  }
  // non-const& overload
  {
    expected<Val, Err> e(std::in_place, 2);
    Val& val = *e;
    ASSERT_EQ(val.x, 2);
    ASSERT_EQ((*e).x, 2);
    val.x = 20;
    ASSERT_EQ(val.x, 20);
    ASSERT_EQ((*e).x, 20);
  }
  // const&& overload
  {
    const expected<Val, Err> e(std::in_place, 3);
    Val val = *std::move(e);
    ASSERT_EQ(val.x, 3);
    ASSERT_EQ((*e).x, 3);
    // Since the r-value reference is const, Val's copy constructor is called,
    // and not Val's move constructor (which takes a non-const r-value
    // reference).
  }
  {
    const expected<Val, Err> e(std::in_place, 4);
    Val val;
    val = *std::move(e);
    ASSERT_EQ(val.x, 4);
    ASSERT_EQ((*e).x, 4);
    // Since the r-value reference is const, Val's copy assignment is called,
    // and not Val's move assignment (which takes a non-const r-value
    // reference).
  }
  // non-const&& overload
  {
    expected<Val, Err> e(std::in_place, 5);
    Val val = *std::move(e);
    ASSERT_EQ(val.x, 5);
    ASSERT_EQ((*e).x, -1);
  }
  {
    expected<Val, Err> e(std::in_place, 6);
    Val val;
    val = *std::move(e);
    ASSERT_EQ(val.x, 6);
    ASSERT_EQ((*e).x, -2);
  }
}

TEST(expected, error) {
  // const& overload
  {
    const expected<Val, Err> e(unexpect, 1);
    const Err& err = e.error();
    ASSERT_EQ(err.x, 1);
    ASSERT_EQ(e.error().x, 1);
  }
  // non-const& overload
  {
    expected<Val, Err> e(unexpect, 2);
    Err& err = e.error();
    ASSERT_EQ(err.x, 2);
    ASSERT_EQ(e.error().x, 2);
    err.x = 20;
    ASSERT_EQ(err.x, 20);
    ASSERT_EQ(e.error().x, 20);
  }
  // const&& overload
  {
    const expected<Val, Err> e(unexpect, 3);
    Err err = std::move(e).error();
    ASSERT_EQ(err.x, 3);
    ASSERT_EQ(e.error().x, 3);
    // Since the r-value reference is const, Err's copy constructor is called,
    // and not Err's move constructor (which takes a non-const r-value
    // reference).
  }
  {
    const expected<Val, Err> e(unexpect, 4);
    Err err;
    err = std::move(e).error();
    ASSERT_EQ(err.x, 4);
    ASSERT_EQ(e.error().x, 4);
    // Since the r-value reference is const, Err's copy assignment is called,
    // and not Err's move assignment (which takes a non-const r-value
    // reference).
  }
  // non-const&& overload
  {
    expected<Val, Err> e(unexpect, 5);
    Err err = std::move(e).error();
    ASSERT_EQ(err.x, 5);
    ASSERT_EQ(e.error().x, -1);
  }
  {
    expected<Val, Err> e(unexpect, 6);
    Err err;
    err = std::move(e).error();
    ASSERT_EQ(err.x, 6);
    ASSERT_EQ(e.error().x, -2);
  }
}

TEST(expected, has_value) {
  {
    expected<Val, Err> e;
    ASSERT_TRUE(static_cast<bool>(e));
    ASSERT_TRUE(e.has_value());
  }
  {
    expected<Val, Err> e(std::in_place, Arg(1), 1);
    ASSERT_TRUE(static_cast<bool>(e));
    ASSERT_TRUE(e.has_value());
  }
  {
    expected<Val, Err> e(std::in_place, {2}, Arg(2), 2);
    ASSERT_TRUE(static_cast<bool>(e));
    ASSERT_TRUE(e.has_value());
  }
  {
    expected<Val, Err> e(unexpect, Arg(3), 3);
    ASSERT_FALSE(static_cast<bool>(e));
    ASSERT_FALSE(e.has_value());
  }
  {
    expected<Val, Err> e(unexpect, {4}, Arg(4), 4);
    ASSERT_FALSE(static_cast<bool>(e));
    ASSERT_FALSE(e.has_value());
  }
}

TEST(expected, default_constructor) {
  Val::reset();
  Err::reset();
  {
    expected<Val, Err> e;
    ASSERT_EQ(Val::s, State::default_constructed);
    ASSERT_EQ(Err::s, State::none);
    ASSERT_EQ(e->x, 0);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
}
