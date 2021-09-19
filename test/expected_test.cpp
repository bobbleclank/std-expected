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
  ~Arg() = default;

  Arg(const Arg&) = default;
  Arg& operator=(const Arg&) = default;

  Arg(Arg&& other) {
    x = other.x;
    other.x = -1;
  }

  Arg& operator=(Arg&& other) {
    x = other.x;
    other.x = -2;
    return *this;
  }

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

  Obj(Arg&& arg_, int) {
    s = State::args_constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  Obj(std::initializer_list<int>, const Arg& arg_, int) {
    s = State::list_constructed;
    Arg arg = arg_;
    x = arg.x;
  }

  Obj(std::initializer_list<int>, Arg&& arg_, int) {
    s = State::list_constructed;
    Arg arg = std::move(arg_);
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

struct Con {
  Con() = default;
  explicit Con(int x_) : x(x_) {}

  operator Val() const& { return Val(x); }

  operator Val() && {
    Val val(x);
    x = -3;
    return val;
  }

  int x = 0;
};

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

TEST(expected, value) {
  // const& overload
  {
    const expected<Val, Err> e(std::in_place, 1);
    const Val& val = e.value();
    ASSERT_EQ(val.x, 1);
    ASSERT_EQ(e.value().x, 1);
  }
  {
    const expected<Val, Err> e(unexpect, 2);
    bool did_throw = false;
    try {
      e.value();
    } catch (const bad_expected_access<Err>& ex) {
      ASSERT_EQ(ex.error().x, 2);
      did_throw = true;
    }
    ASSERT_EQ(e.error().x, 2);
    ASSERT_TRUE(did_throw);
  }
  // non-const& overload
  {
    expected<Val, Err> e(std::in_place, 3);
    Val& val = e.value();
    ASSERT_EQ(val.x, 3);
    ASSERT_EQ(e.value().x, 3);
    val.x = 30;
    ASSERT_EQ(val.x, 30);
    ASSERT_EQ(e.value().x, 30);
  }
  {
    expected<Val, Err> e(unexpect, 4);
    bool did_throw = false;
    try {
      e.value();
    } catch (const bad_expected_access<Err>& ex) {
      ASSERT_EQ(ex.error().x, 4);
      did_throw = true;
    }
    ASSERT_EQ(e.error().x, 4);
    ASSERT_TRUE(did_throw);
  }
  // const&& overload
  {
    const expected<Val, Err> e(std::in_place, 5);
    Val val = std::move(e).value();
    ASSERT_EQ(val.x, 5);
    ASSERT_EQ(e.value().x, 5);
    // Since the r-value reference is const, Val's copy constructor is called,
    // and not Val's move constructor (which takes a non-const r-value
    // reference).
  }
  {
    const expected<Val, Err> e(std::in_place, 6);
    Val val;
    val = std::move(e).value();
    ASSERT_EQ(val.x, 6);
    ASSERT_EQ(e.value().x, 6);
    // Since the r-value reference is const, Val's copy assignment is called,
    // and not Val's move assignment (which takes a non-const r-value
    // reference).
  }
  {
    const expected<Val, Err> e(unexpect, 7);
    bool did_throw = false;
    try {
      std::move(e).value();
    } catch (const bad_expected_access<Err>& ex) {
      ASSERT_EQ(ex.error().x, 7);
      did_throw = true;
    }
    ASSERT_EQ(e.error().x, 7);
    ASSERT_TRUE(did_throw);
    // Since the r-value reference is const, Err's copy constructor is called,
    // and not Err's move constructor (which takes a non-const r-value
    // reference).
  }
  // non-const&& overload
  {
    expected<Val, Err> e(std::in_place, 8);
    Val val = std::move(e).value();
    ASSERT_EQ(val.x, 8);
    ASSERT_EQ(e.value().x, -1);
  }
  {
    expected<Val, Err> e(std::in_place, 9);
    Val val;
    val = std::move(e).value();
    ASSERT_EQ(val.x, 9);
    ASSERT_EQ(e.value().x, -2);
  }
  {
    expected<Val, Err> e(unexpect, 10);
    bool did_throw = false;
    try {
      std::move(e).value();
    } catch (const bad_expected_access<Err>& ex) {
      ASSERT_EQ(ex.error().x, 10);
      did_throw = true;
    }
    ASSERT_EQ(e.error().x, -1);
    ASSERT_TRUE(did_throw);
  }
}

TEST(expected, value_or) {
  // const& overload
  {
    expected<Val, Err> e(std::in_place, 1);
    Val v(10);
    Val val = e.value_or(v);
    ASSERT_EQ(val.x, 1);
    ASSERT_EQ(e->x, 1);
    ASSERT_EQ(v.x, 10);
  }
  {
    expected<Val, Err> e(unexpect, 2);
    Val v(20);
    Val val = e.value_or(v);
    ASSERT_EQ(val.x, 20);
    ASSERT_EQ(e.error().x, 2);
    ASSERT_EQ(v.x, 20);
  }
  {
    expected<Val, Err> e(unexpect, 3);
    Val v(30);
    Val val = e.value_or(std::move(v));
    ASSERT_EQ(val.x, 30);
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(v.x, -1);
  }
  {
    expected<Val, Err> e(unexpect, 4);
    Con c(40);
    Val val = e.value_or(c);
    ASSERT_EQ(val.x, 40);
    ASSERT_EQ(e.error().x, 4);
    ASSERT_EQ(c.x, 40);
  }
  {
    expected<Val, Err> e(unexpect, 5);
    Con c(50);
    Val val = e.value_or(std::move(c));
    ASSERT_EQ(val.x, 50);
    ASSERT_EQ(e.error().x, 5);
    ASSERT_EQ(c.x, -3);
  }
  // non-const&& overload
  {
    expected<Val, Err> e(std::in_place, 6);
    Val v(60);
    Val val = std::move(e).value_or(v);
    ASSERT_EQ(val.x, 6);
    ASSERT_EQ(e->x, -1);
    ASSERT_EQ(v.x, 60);
  }
  {
    expected<Val, Err> e(unexpect, 7);
    Val v(70);
    Val val = std::move(e).value_or(v);
    ASSERT_EQ(val.x, 70);
    ASSERT_EQ(e.error().x, 7);
    ASSERT_EQ(v.x, 70);
  }
  {
    expected<Val, Err> e(unexpect, 8);
    Val v(80);
    Val val = std::move(e).value_or(std::move(v));
    ASSERT_EQ(val.x, 80);
    ASSERT_EQ(e.error().x, 8);
    ASSERT_EQ(v.x, -1);
  }
  {
    expected<Val, Err> e(unexpect, 9);
    Con c(90);
    Val val = std::move(e).value_or(c);
    ASSERT_EQ(val.x, 90);
    ASSERT_EQ(e.error().x, 9);
    ASSERT_EQ(c.x, 90);
  }
  {
    expected<Val, Err> e(unexpect, 10);
    Con c(100);
    Val val = std::move(e).value_or(std::move(c));
    ASSERT_EQ(val.x, 100);
    ASSERT_EQ(e.error().x, 10);
    ASSERT_EQ(c.x, -3);
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

TEST(expected, args_constructor) {
  Val::reset();
  Err::reset();
  // in_place overload
  {
    Arg arg(1);
    expected<Val, Err> e(std::in_place, arg, 1);
    ASSERT_EQ(Val::s, State::args_constructed);
    ASSERT_EQ(Err::s, State::none);
    ASSERT_EQ(e->x, 1);
    ASSERT_EQ(arg.x, 1);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  {
    Arg arg(2);
    expected<Val, Err> e(std::in_place, std::move(arg), 2);
    ASSERT_EQ(Val::s, State::args_constructed);
    ASSERT_EQ(Err::s, State::none);
    ASSERT_EQ(e->x, 2);
    ASSERT_EQ(arg.x, -1);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  // unexpect overload
  {
    Arg arg(3);
    expected<Val, Err> e(unexpect, arg, 3);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::args_constructed);
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(arg.x, 3);
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(4);
    expected<Val, Err> e(unexpect, std::move(arg), 4);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::args_constructed);
    ASSERT_EQ(e.error().x, 4);
    ASSERT_EQ(arg.x, -1);
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
}

TEST(expected, list_constructor) {
  Val::reset();
  Err::reset();
  // in_place overload
  {
    Arg arg(1);
    expected<Val, Err> e(std::in_place, {1}, arg, 1);
    ASSERT_EQ(Val::s, State::list_constructed);
    ASSERT_EQ(Err::s, State::none);
    ASSERT_EQ(e->x, 1);
    ASSERT_EQ(arg.x, 1);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  {
    Arg arg(2);
    expected<Val, Err> e(std::in_place, {2}, std::move(arg), 2);
    ASSERT_EQ(Val::s, State::list_constructed);
    ASSERT_EQ(Err::s, State::none);
    ASSERT_EQ(e->x, 2);
    ASSERT_EQ(arg.x, -1);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  // unexpect overload
  {
    Arg arg(3);
    expected<Val, Err> e(unexpect, {3}, arg, 3);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::list_constructed);
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(arg.x, 3);
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(4);
    expected<Val, Err> e(unexpect, {4}, std::move(arg), 4);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::list_constructed);
    ASSERT_EQ(e.error().x, 4);
    ASSERT_EQ(arg.x, -1);
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
}
