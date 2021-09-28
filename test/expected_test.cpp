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

  Arg(Arg&& other) noexcept {
    x = other.x;
    other.x = -1;
  }

  Arg& operator=(Arg&& other) noexcept {
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

  Obj(Obj&& other) noexcept {
    s = State::move_constructed;
    x = other.x;
    other.x = -1;
  }

  Obj& operator=(const Obj& other) {
    s = State::copy_assigned;
    x = other.x;
    return *this;
  }

  Obj& operator=(Obj&& other) noexcept {
    s = State::move_assigned;
    x = other.x;
    other.x = -2;
    return *this;
  }

  ~Obj() { s = State::destructed; }

  int x = 0;
};

template <class Tag> bool operator==(Obj<Tag> lhs, Obj<Tag> rhs) {
  return lhs.x == rhs.x;
}

template <class Tag> bool operator!=(Obj<Tag> lhs, Obj<Tag> rhs) {
  return !(lhs == rhs);
}

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

struct Val2_tag {};
using Val2 = Obj<Val2_tag>;

bool operator==(Val2 lhs, Val rhs) { return lhs.x == rhs.x; }
bool operator!=(Val2 lhs, Val rhs) { return !(lhs == rhs); }

struct Err2_tag {};
using Err2 = Obj<Err2_tag>;

bool operator==(Err2 lhs, Err rhs) { return lhs.x == rhs.x; }
bool operator!=(Err2 lhs, Err rhs) { return !(lhs == rhs); }

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

TEST(expected, copy_constructor) {
  Val::reset();
  Err::reset();
  // (const expected&)
  {
    expected<Val, Err> other(std::in_place, 1);
    Val::reset();
    {
      expected<Val, Err> e(other);
      ASSERT_EQ(Val::s, State::copy_constructed);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
      ASSERT_EQ(e->x, 1);
      ASSERT_EQ(other->x, 1);
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  {
    expected<Val, Err> other(unexpect, 2);
    Err::reset();
    {
      expected<Val, Err> e(other);
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::copy_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(other.error().x, 2);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  // (expected&&)
  {
    expected<Val, Err> other(std::in_place, 3);
    Val::reset();
    {
      expected<Val, Err> e(std::move(other));
      ASSERT_EQ(Val::s, State::move_constructed);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
      ASSERT_EQ(e->x, 3);
      ASSERT_EQ(other->x, -1);
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  {
    expected<Val, Err> other(unexpect, 4);
    Err::reset();
    {
      expected<Val, Err> e(std::move(other));
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::move_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 4);
      ASSERT_EQ(other.error().x, -1);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
}

TEST(expected, variadic_template_constructor) {
  Val::reset();
  Err::reset();
  // (std::in_place_t, Args&&...)
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
  // (unexpect_t, Args&&...)
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
  // (std::in_place_t, std::initializer_list<U>, Args&&...)
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
  // (unexpect_t, std::initializer_list<U>, Args&&...)
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

TEST(expected, copy_assignment) {
  Val::reset();
  Err::reset();
  // (const expected&)
  {
    expected<Val, Err> other(std::in_place, 1);
    Val::reset();
    {
      expected<Val, Err> e(std::in_place, 10);
      e = other;
      ASSERT_EQ(Val::s, State::copy_assigned);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
      ASSERT_EQ(e->x, 1);
      ASSERT_EQ(other->x, 1);
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  {
    expected<Val, Err> other(std::in_place, 2);
    Val::reset();
    {
      expected<Val, Err> e(unexpect, 20);
      e = other;
      ASSERT_EQ(Val::s, State::copy_constructed);
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
      ASSERT_EQ(e->x, 2);
      ASSERT_EQ(other->x, 2);
      Err::reset();
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  {
    expected<Val, Err> other(unexpect, 3);
    Err::reset();
    {
      expected<Val, Err> e(unexpect, 30);
      e = other;
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::copy_assigned);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 3);
      ASSERT_EQ(other.error().x, 3);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  {
    expected<Val, Err> other(unexpect, 4);
    Err::reset();
    {
      expected<Val, Err> e(std::in_place, 40);
      e = other;
      ASSERT_EQ(Val::s, State::destructed);
      ASSERT_EQ(Err::s, State::copy_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 4);
      ASSERT_EQ(other.error().x, 4);
      Val::reset();
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  // (expected&&)
  {
    expected<Val, Err> other(std::in_place, 5);
    Val::reset();
    {
      expected<Val, Err> e(std::in_place, 50);
      e = std::move(other);
      ASSERT_EQ(Val::s, State::move_assigned);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
      ASSERT_EQ(e->x, 5);
      ASSERT_EQ(other->x, -2);
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  {
    expected<Val, Err> other(std::in_place, 6);
    Val::reset();
    {
      expected<Val, Err> e(unexpect, 60);
      e = std::move(other);
      ASSERT_EQ(Val::s, State::move_constructed);
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
      ASSERT_EQ(e->x, 6);
      ASSERT_EQ(other->x, -1);
      Err::reset();
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  {
    expected<Val, Err> other(unexpect, 7);
    Err::reset();
    {
      expected<Val, Err> e(unexpect, 70);
      e = std::move(other);
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::move_assigned);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 7);
      ASSERT_EQ(other.error().x, -2);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  {
    expected<Val, Err> other(unexpect, 8);
    Err::reset();
    {
      expected<Val, Err> e(std::in_place, 80);
      e = std::move(other);
      ASSERT_EQ(Val::s, State::destructed);
      ASSERT_EQ(Err::s, State::move_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 8);
      ASSERT_EQ(other.error().x, -1);
      Val::reset();
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
}

TEST(expected, equality_operators) {
  expected<Val, Err> e_one(std::in_place, 1);
  expected<Val, Err> u_one(unexpect, 1);

  expected<Val, Err> e1(std::in_place, 1);
  expected<Val, Err> e2(std::in_place, 2);

  expected<Val, Err> u1(unexpect, 1);
  expected<Val, Err> u2(unexpect, 2);

  expected<Val2, Err2> e_two(std::in_place, 2);
  expected<Val2, Err2> u_two(unexpect, 2);

  // Operands have same type.

  ASSERT_TRUE(e_one == e1);
  ASSERT_FALSE(e_one == e2);
  ASSERT_FALSE(e_one != e1);
  ASSERT_TRUE(e_one != e2);

  ASSERT_TRUE(u_one == u1);
  ASSERT_FALSE(u_one == u2);
  ASSERT_FALSE(u_one != u1);
  ASSERT_TRUE(u_one != u2);

  ASSERT_FALSE(e_one == u1);
  ASSERT_FALSE(e_one == u2);
  ASSERT_TRUE(e_one != u1);
  ASSERT_TRUE(e_one != u2);

  ASSERT_FALSE(u_one == e1);
  ASSERT_FALSE(u_one == e2);
  ASSERT_TRUE(u_one != e1);
  ASSERT_TRUE(u_one != e2);

  // Operands have different types.

  ASSERT_FALSE(e_two == e1);
  ASSERT_TRUE(e_two == e2);
  ASSERT_TRUE(e_two != e1);
  ASSERT_FALSE(e_two != e2);

  ASSERT_FALSE(u_two == u1);
  ASSERT_TRUE(u_two == u2);
  ASSERT_TRUE(u_two != u1);
  ASSERT_FALSE(u_two != u2);

  ASSERT_FALSE(e_two == u1);
  ASSERT_FALSE(e_two == u2);
  ASSERT_TRUE(e_two != u1);
  ASSERT_TRUE(e_two != u2);

  ASSERT_FALSE(u_two == e1);
  ASSERT_FALSE(u_two == e2);
  ASSERT_TRUE(u_two != e1);
  ASSERT_TRUE(u_two != e2);
}

TEST(expected, comparison_with_T) {
  expected<Val, Err> e_one(std::in_place, 1);
  expected<Val, Err> u_one(unexpect, 1);

  Val v1(1);
  Val v2(2);

  expected<Val2, Err2> e_two(std::in_place, 2);
  expected<Val2, Err2> u_two(unexpect, 2);

  // expected::value_type and T have same type.

  ASSERT_TRUE(e_one == v1);
  ASSERT_FALSE(e_one == v2);
  ASSERT_FALSE(e_one != v1);
  ASSERT_TRUE(e_one != v2);

  ASSERT_FALSE(u_one == v1);
  ASSERT_FALSE(u_one == v2);
  ASSERT_TRUE(u_one != v1);
  ASSERT_TRUE(u_one != v2);

  ASSERT_TRUE(v1 == e_one);
  ASSERT_FALSE(v2 == e_one);
  ASSERT_FALSE(v1 != e_one);
  ASSERT_TRUE(v2 != e_one);

  ASSERT_FALSE(v1 == u_one);
  ASSERT_FALSE(v2 == u_one);
  ASSERT_TRUE(v1 != u_one);
  ASSERT_TRUE(v2 != u_one);

  // expected::value_type and T have different types.

  ASSERT_FALSE(e_two == v1);
  ASSERT_TRUE(e_two == v2);
  ASSERT_TRUE(e_two != v1);
  ASSERT_FALSE(e_two != v2);

  ASSERT_FALSE(u_two == v1);
  ASSERT_FALSE(u_two == v2);
  ASSERT_TRUE(u_two != v1);
  ASSERT_TRUE(u_two != v2);

  ASSERT_FALSE(v1 == e_two);
  ASSERT_TRUE(v2 == e_two);
  ASSERT_TRUE(v1 != e_two);
  ASSERT_FALSE(v2 != e_two);

  ASSERT_FALSE(v1 == u_two);
  ASSERT_FALSE(v2 == u_two);
  ASSERT_TRUE(v1 != u_two);
  ASSERT_TRUE(v2 != u_two);
}

TEST(expected, comparison_with_unexpected_E) {
  expected<Val, Err> e_one(std::in_place, 1);
  expected<Val, Err> u_one(unexpect, 1);

  unexpected<Err> v1(1);
  unexpected<Err> v2(2);

  expected<Val2, Err2> e_two(std::in_place, 2);
  expected<Val2, Err2> u_two(unexpect, 2);

  // expected::error_type and E have same type.

  ASSERT_TRUE(u_one == v1);
  ASSERT_FALSE(u_one == v2);
  ASSERT_FALSE(u_one != v1);
  ASSERT_TRUE(u_one != v2);

  ASSERT_FALSE(e_one == v1);
  ASSERT_FALSE(e_one == v2);
  ASSERT_TRUE(e_one != v1);
  ASSERT_TRUE(e_one != v2);

  ASSERT_TRUE(v1 == u_one);
  ASSERT_FALSE(v2 == u_one);
  ASSERT_FALSE(v1 != u_one);
  ASSERT_TRUE(v2 != u_one);

  ASSERT_FALSE(v1 == e_one);
  ASSERT_FALSE(v2 == e_one);
  ASSERT_TRUE(v1 != e_one);
  ASSERT_TRUE(v2 != e_one);

  // expected::error_type and E have different types.

  ASSERT_FALSE(u_two == v1);
  ASSERT_TRUE(u_two == v2);
  ASSERT_TRUE(u_two != v1);
  ASSERT_FALSE(u_two != v2);

  ASSERT_FALSE(e_two == v1);
  ASSERT_FALSE(e_two == v2);
  ASSERT_TRUE(e_two != v1);
  ASSERT_TRUE(e_two != v2);

  ASSERT_FALSE(v1 == u_two);
  ASSERT_TRUE(v2 == u_two);
  ASSERT_TRUE(v1 != u_two);
  ASSERT_FALSE(v2 != u_two);

  ASSERT_FALSE(v1 == e_two);
  ASSERT_FALSE(v2 == e_two);
  ASSERT_TRUE(v1 != e_two);
  ASSERT_TRUE(v2 != e_two);
}
