#include "bc/exp/expected.h"

#include "arg.h"
#include "con.h"
#include "obj.h"
#include "obj_implicit.h"
#include "obj_throw.h"
#include "state.h"

#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

using namespace bc::exp;

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
    Val val(40);
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
    Val val(60);
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
    Err err(40);
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
    Err err(60);
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
    Val val(60);
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
    Val val(90);
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
    expected<Val, Err> e(std::in_place);
    ASSERT_TRUE(static_cast<bool>(e));
    ASSERT_TRUE(e.has_value());
  }
  {
    expected<Val, Err> e(unexpect);
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
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 20100);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
}

TEST(expected, copy_constructor) {
  Val::reset();
  Err::reset();
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
}

TEST(expected, move_constructor) {
  Val::reset();
  Err::reset();
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

TEST(expected, copy_expected_constructor) {
  Val::reset();
  Err::reset();
  Val_implicit::reset();
  Err_implicit::reset();
  // explicit
  {
    expected<Arg, Arg> other(std::in_place, 1);
    {
      expected<Val, Err> e(other);
      ASSERT_EQ(Val::s, State::constructed);
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
  {
    expected<Arg, Arg> other(unexpect, 2);
    {
      expected<Val, Err> e(other);
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(other.error().x, 2);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  // implicit
  {
    expected<Arg, Arg> other(std::in_place, 3);
    {
      expected<Val_implicit, Err_implicit> e = other;
      ASSERT_EQ(Val_implicit::s, State::constructed);
      ASSERT_EQ(Err_implicit::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
      ASSERT_EQ(e->x, 3);
      ASSERT_EQ(other->x, 3);
    }
    ASSERT_EQ(Val_implicit::s, State::destructed);
    ASSERT_EQ(Err_implicit::s, State::none);
    Val_implicit::reset();
  }
  {
    expected<Arg, Arg> other(unexpect, 4);
    {
      expected<Val_implicit, Err_implicit> e = other;
      ASSERT_EQ(Val_implicit::s, State::none);
      ASSERT_EQ(Err_implicit::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 4);
      ASSERT_EQ(other.error().x, 4);
    }
    ASSERT_EQ(Val_implicit::s, State::none);
    ASSERT_EQ(Err_implicit::s, State::destructed);
    Err_implicit::reset();
  }
}

TEST(expected, move_expected_constructor) {
  Val::reset();
  Err::reset();
  Val_implicit::reset();
  Err_implicit::reset();
  // explicit
  {
    expected<Arg, Arg> other(std::in_place, 1);
    {
      expected<Val, Err> e(std::move(other));
      ASSERT_EQ(Val::s, State::constructed);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
      ASSERT_EQ(e->x, 1);
      ASSERT_EQ(other->x, -1);
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  {
    expected<Arg, Arg> other(unexpect, 2);
    {
      expected<Val, Err> e(std::move(other));
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(other.error().x, -1);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  // implicit
  {
    expected<Arg, Arg> other(std::in_place, 3);
    {
      expected<Val_implicit, Err_implicit> e = std::move(other);
      ASSERT_EQ(Val_implicit::s, State::constructed);
      ASSERT_EQ(Err_implicit::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
      ASSERT_EQ(e->x, 3);
      ASSERT_EQ(other->x, -1);
    }
    ASSERT_EQ(Val_implicit::s, State::destructed);
    ASSERT_EQ(Err_implicit::s, State::none);
    Val_implicit::reset();
  }
  {
    expected<Arg, Arg> other(unexpect, 4);
    {
      expected<Val_implicit, Err_implicit> e = std::move(other);
      ASSERT_EQ(Val_implicit::s, State::none);
      ASSERT_EQ(Err_implicit::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 4);
      ASSERT_EQ(other.error().x, -1);
    }
    ASSERT_EQ(Val_implicit::s, State::none);
    ASSERT_EQ(Err_implicit::s, State::destructed);
    Err_implicit::reset();
  }
}

TEST(expected, value_constructor) {
  Val::reset();
  Err::reset();
  Val_implicit::reset();
  // explicit with U = T
  {
    Val val(1);
    Val::reset();
    {
      expected<Val, Err> e(std::move(val));
      ASSERT_EQ(Val::s, State::move_constructed);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_EQ(e->x, 1);
      ASSERT_EQ(val.x, -1);
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  // explicit with U != T
  {
    Arg val(2);
    {
      expected<Val, Err> e(std::move(val));
      ASSERT_EQ(Val::s, State::constructed);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_EQ(e->x, 2);
      ASSERT_EQ(val.x, -1);
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  // implicit with U = T
  {
    Val_implicit val(3);
    Val_implicit::reset();
    {
      expected<Val_implicit, Err> e = std::move(val);
      ASSERT_EQ(Val_implicit::s, State::move_constructed);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_EQ(e->x, 3);
      ASSERT_EQ(val.x, -1);
    }
    ASSERT_EQ(Val_implicit::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val_implicit::reset();
  }
  Val_implicit::reset();
  // implicit with U != T
  {
    Arg val(4);
    {
      expected<Val_implicit, Err> e = std::move(val);
      ASSERT_EQ(Val_implicit::s, State::constructed);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_EQ(e->x, 4);
      ASSERT_EQ(val.x, -1);
    }
    ASSERT_EQ(Val_implicit::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val_implicit::reset();
  }
}

TEST(expected, copy_unexpected_constructor) {
  Val::reset();
  Err::reset();
  Err_implicit::reset();
  // explicit with G = E
  {
    unexpected val(Err(1));
    Err::reset();
    {
      expected<Val, Err> e(val);
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::copy_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(val.value().x, 1);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  // explicit with G != E
  {
    unexpected val(Arg(2));
    {
      expected<Val, Err> e(val);
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(val.value().x, 2);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  // implicit with G = E
  {
    unexpected val(Err_implicit(3));
    Err_implicit::reset();
    {
      expected<Val, Err_implicit> e = val;
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err_implicit::s, State::copy_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 3);
      ASSERT_EQ(val.value().x, 3);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_implicit::s, State::destructed);
    Err_implicit::reset();
  }
  Err_implicit::reset();
  // implicit with G != E
  {
    unexpected val(Arg(4));
    {
      expected<Val, Err_implicit> e = val;
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err_implicit::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 4);
      ASSERT_EQ(val.value().x, 4);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_implicit::s, State::destructed);
    Err_implicit::reset();
  }
}

TEST(expected, move_unexpected_constructor) {
  Val::reset();
  Err::reset();
  Err_implicit::reset();
  // explicit with G = E
  {
    unexpected val(Err(1));
    Err::reset();
    {
      expected<Val, Err> e(std::move(val));
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::move_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(val.value().x, -1);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  // explicit with G != E
  {
    unexpected val(Arg(2));
    {
      expected<Val, Err> e(std::move(val));
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(val.value().x, -1);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  // implicit with G = E
  {
    unexpected val(Err_implicit(3));
    Err_implicit::reset();
    {
      expected<Val, Err_implicit> e = std::move(val);
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err_implicit::s, State::move_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 3);
      ASSERT_EQ(val.value().x, -1);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_implicit::s, State::destructed);
    Err_implicit::reset();
  }
  Err_implicit::reset();
  // implicit with G != E
  {
    unexpected val(Arg(4));
    {
      expected<Val, Err_implicit> e = std::move(val);
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err_implicit::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 4);
      ASSERT_EQ(val.value().x, -1);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_implicit::s, State::destructed);
    Err_implicit::reset();
  }
}

TEST(expected, in_place_constructor) {
  Val::reset();
  Err::reset();
  // (std::in_place_t, Args&&...)
  {
    expected<Val, Err> e(std::in_place);
    ASSERT_EQ(Val::s, State::default_constructed);
    ASSERT_EQ(Err::s, State::none);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 20100);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  {
    Arg arg(2);
    expected<Val, Err> e(std::in_place, std::move(arg), 2);
    ASSERT_EQ(Val::s, State::constructed);
    ASSERT_EQ(Err::s, State::none);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 2 + 2);
    ASSERT_EQ(arg.x, -1);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  // (unexpect_t, Args&&...)
  {
    expected<Val, Err> e(unexpect);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::default_constructed);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 20100);
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(4);
    expected<Val, Err> e(unexpect, std::move(arg), 4);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::constructed);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 4 + 4);
    ASSERT_EQ(arg.x, -1);
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  // (std::in_place_t, std::initializer_list<U>, Args&&...)
  {
    Arg arg(2);
    expected<Val, Err> e(std::in_place, {2}, std::move(arg), 2);
    ASSERT_EQ(Val::s, State::constructed);
    ASSERT_EQ(Err::s, State::none);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 2 + 2 + 2);
    ASSERT_EQ(arg.x, -1);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  // (unexpect_t, std::initializer_list<U>, Args&&...)
  {
    Arg arg(4);
    expected<Val, Err> e(unexpect, {4}, std::move(arg), 4);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::constructed);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 4 + 4 + 4);
    ASSERT_EQ(arg.x, -1);
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
}

TEST(expected, copy_assignment_operator) {
  Val::reset();
  Err::reset();
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
}

TEST(expected, move_assignment_operator) {
  Val::reset();
  Err::reset();
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

TEST(expected, value_assignment_operator) {
  Val::reset();
  Err::reset();
  Val_throw_2::reset();
  // has_value() with U = T
  {
    Val val(1);
    Val::reset();
    {
      expected<Val, Err> e(std::in_place, 10);
      e = std::move(val);
      ASSERT_EQ(Val::s, State::move_assigned);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_EQ(e->x, 1);
      ASSERT_EQ(val.x, -2);
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  {
    Val_throw_2 val(2);
    Val_throw_2::reset();
    {
      expected<Val_throw_2, Err> e(std::in_place, 20);
      bool did_throw = false;
      try {
        Val_throw_2::t = May_throw::do_throw;
        e = std::move(val);
      } catch (...) {
        ASSERT_EQ(Val_throw_2::s, State::move_assigned); // failed
        ASSERT_EQ(Err::s, State::none);
        did_throw = true;
        Val_throw_2::t = May_throw::do_not_throw;
      }
      ASSERT_TRUE(e.has_value());
      ASSERT_EQ(e->x, 2); // No exception safety guarantee.
      ASSERT_EQ(val.x, -2);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Val_throw_2::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val_throw_2::reset();
  }
  Val_throw_2::reset();
  // !has_value() via std::is_nothrow_constructible_v<T, U&&> with U = T
  {
    Val val(3);
    Val::reset();
    {
      expected<Val, Err> e(unexpect, 30);
      e = std::move(val);
      ASSERT_EQ(Val::s, State::move_constructed);
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(e.has_value());
      ASSERT_EQ(e->x, 3);
      ASSERT_EQ(val.x, -1);
      Err::reset();
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  // !has_value() via std::is_nothrow_move_constructible_v<E> with U = T
  {
    Val_throw_2 val(4);
    Val_throw_2::reset();
    {
      expected<Val_throw_2, Err> e(unexpect, 40);
      e = std::move(val);
      ASSERT_EQ(Val_throw_2::s, State::move_constructed);
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(e.has_value());
      ASSERT_EQ(e->x, 4);
      ASSERT_EQ(val.x, -1);
      Err::reset();
    }
    ASSERT_EQ(Val_throw_2::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val_throw_2::reset();
  }
  Val_throw_2::reset();
  {
    Val_throw_2 val(5);
    Val_throw_2::reset();
    {
      expected<Val_throw_2, Err> e(unexpect, 50);
      bool did_throw = false;
      try {
        Val_throw_2::t = May_throw::do_throw;
        e = std::move(val);
      } catch (...) {
        ASSERT_EQ(Val_throw_2::s, State::move_constructed); // failed
        // move_constructed (tmp), move_constructed (this), destructed (tmp)
        ASSERT_EQ(Err::s, State::destructed);
        did_throw = true;
        Val_throw_2::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 50);
      ASSERT_EQ(val.x, -1);
      ASSERT_TRUE(did_throw);
      Val_throw_2::reset();
      Err::s = State::constructed;
    }
    ASSERT_EQ(Val_throw_2::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Val_throw_2::reset();
  // has_value() with U != T
  {
    Arg val(1);
    {
      expected<Val, Err> e(std::in_place, 10);
      e = std::move(val);
      ASSERT_EQ(Val::s, State::assigned);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_EQ(e->x, 1);
      ASSERT_EQ(val.x, -1);
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  {
    Arg val(2);
    {
      expected<Val_throw_2, Err> e(std::in_place, 20);
      bool did_throw = false;
      try {
        Val_throw_2::t = May_throw::do_throw;
        e = std::move(val);
      } catch (...) {
        ASSERT_EQ(Val_throw_2::s, State::assigned); // failed
        ASSERT_EQ(Err::s, State::none);
        did_throw = true;
        Val_throw_2::t = May_throw::do_not_throw;
      }
      ASSERT_TRUE(e.has_value());
      ASSERT_EQ(e->x, 2); // No exception safety guarantee.
      ASSERT_EQ(val.x, -1);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Val_throw_2::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val_throw_2::reset();
  }
  // !has_value() via std::is_nothrow_constructible_v<T, U&&> with U != T
  {
    Arg val(3);
    {
      expected<Val, Err> e(unexpect, 30);
      e = std::move(val);
      ASSERT_EQ(Val::s, State::constructed);
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(e.has_value());
      ASSERT_EQ(e->x, 3);
      ASSERT_EQ(val.x, -1);
      Err::reset();
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  // !has_value() via std::is_nothrow_move_constructible_v<E> with U != T
  {
    Arg val(4);
    {
      expected<Val_throw_2, Err> e(unexpect, 40);
      e = std::move(val);
      ASSERT_EQ(Val_throw_2::s, State::constructed);
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(e.has_value());
      ASSERT_EQ(e->x, 4);
      ASSERT_EQ(val.x, -1);
      Err::reset();
    }
    ASSERT_EQ(Val_throw_2::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val_throw_2::reset();
  }
  {
    Arg val(5);
    {
      expected<Val_throw_2, Err> e(unexpect, 50);
      bool did_throw = false;
      try {
        Val_throw_2::t = May_throw::do_throw;
        e = std::move(val);
      } catch (...) {
        ASSERT_EQ(Val_throw_2::s, State::constructed); // failed
        // move_constructed (tmp), move_constructed (this), destructed (tmp)
        ASSERT_EQ(Err::s, State::destructed);
        did_throw = true;
        Val_throw_2::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 50);
      ASSERT_EQ(val.x, -1);
      ASSERT_TRUE(did_throw);
      Val_throw_2::reset();
      Err::s = State::constructed;
    }
    ASSERT_EQ(Val_throw_2::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
}

TEST(expected, copy_unexpected_assignment_operator) {
  Val::reset();
  Err::reset();
  Err_throw_3::reset();
  // has_value() with G = E
  {
    unexpected val(Err(1));
    Err::reset();
    {
      expected<Val, Err> e(std::in_place, 10);
      e = val;
      ASSERT_EQ(Val::s, State::destructed);
      ASSERT_EQ(Err::s, State::copy_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(val.value().x, 1);
      Val::reset();
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  // !has_value() with G = E
  {
    unexpected val(Err(2));
    Err::reset();
    {
      expected<Val, Err> e(unexpect, 20);
      e = val;
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::copy_assigned);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(val.value().x, 2);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  {
    unexpected val(Err_throw_3(3));
    Err_throw_3::reset();
    {
      expected<Val, Err_throw_3> e(unexpect, 30);
      bool did_throw = false;
      try {
        Err_throw_3::t = May_throw::do_throw;
        e = val;
      } catch (...) {
        ASSERT_EQ(Val::s, State::none);
        ASSERT_EQ(Err_throw_3::s, State::copy_assigned); // failed
        did_throw = true;
        Err_throw_3::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 3); // No exception safety guarantee.
      ASSERT_EQ(val.value().x, 3);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_throw_3::s, State::destructed);
    Err_throw_3::reset();
  }
  Err_throw_3::reset();
  // has_value() with G != E
  {
    unexpected val(Arg(4));
    {
      expected<Val, Err> e(std::in_place, 40);
      e = val;
      ASSERT_EQ(Val::s, State::destructed);
      ASSERT_EQ(Err::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 4);
      ASSERT_EQ(val.value().x, 4);
      Val::reset();
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  // !has_value() with G != E
  {
    unexpected val(Arg(5));
    {
      expected<Val, Err> e(unexpect, 50);
      e = val;
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::assigned);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 5);
      ASSERT_EQ(val.value().x, 5);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  {
    unexpected val(Arg(6));
    {
      expected<Val, Err_throw_3> e(unexpect, 60);
      bool did_throw = false;
      try {
        Err_throw_3::t = May_throw::do_throw;
        e = val;
      } catch (...) {
        ASSERT_EQ(Val::s, State::none);
        ASSERT_EQ(Err_throw_3::s, State::assigned); // failed
        did_throw = true;
        Err_throw_3::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 6); // No exception safety guarantee.
      ASSERT_EQ(val.value().x, 6);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_throw_3::s, State::destructed);
    Err_throw_3::reset();
  }
}

TEST(expected, move_unexpected_assignment_operator) {
  Val::reset();
  Err::reset();
  Err_throw_3::reset();
  // has_value() with G = E
  {
    unexpected val(Err(1));
    Err::reset();
    {
      expected<Val, Err> e(std::in_place, 10);
      e = std::move(val);
      ASSERT_EQ(Val::s, State::destructed);
      ASSERT_EQ(Err::s, State::move_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(val.value().x, -1);
      Val::reset();
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  // !has_value() with G = E
  {
    unexpected val(Err(2));
    Err::reset();
    {
      expected<Val, Err> e(unexpect, 20);
      e = std::move(val);
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::move_assigned);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(val.value().x, -2);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  {
    unexpected val(Err_throw_3(3));
    Err_throw_3::reset();
    {
      expected<Val, Err_throw_3> e(unexpect, 30);
      bool did_throw = false;
      try {
        Err_throw_3::t = May_throw::do_throw;
        e = std::move(val);
      } catch (...) {
        ASSERT_EQ(Val::s, State::none);
        ASSERT_EQ(Err_throw_3::s, State::move_assigned); // failed
        did_throw = true;
        Err_throw_3::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 3); // No exception safety guarantee.
      ASSERT_EQ(val.value().x, -2);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_throw_3::s, State::destructed);
    Err_throw_3::reset();
  }
  Err_throw_3::reset();
  // has_value() with G != E
  {
    unexpected val(Arg(4));
    {
      expected<Val, Err> e(std::in_place, 40);
      e = std::move(val);
      ASSERT_EQ(Val::s, State::destructed);
      ASSERT_EQ(Err::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 4);
      ASSERT_EQ(val.value().x, -1);
      Val::reset();
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  // !has_value() with G != E
  {
    unexpected val(Arg(5));
    {
      expected<Val, Err> e(unexpect, 50);
      e = std::move(val);
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::assigned);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 5);
      ASSERT_EQ(val.value().x, -1);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  {
    unexpected val(Arg(6));
    {
      expected<Val, Err_throw_3> e(unexpect, 60);
      bool did_throw = false;
      try {
        Err_throw_3::t = May_throw::do_throw;
        e = std::move(val);
      } catch (...) {
        ASSERT_EQ(Val::s, State::none);
        ASSERT_EQ(Err_throw_3::s, State::assigned); // failed
        did_throw = true;
        Err_throw_3::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 6); // No exception safety guarantee.
      ASSERT_EQ(val.value().x, -1);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_throw_3::s, State::destructed);
    Err_throw_3::reset();
  }
}

TEST(expected, emplace) {
  Val::reset();
  Err::reset();
  Val_throw::reset();
  Val_throw_2::reset();
  // has_value()
  {
    Arg arg(2);
    expected<Val, Err> e(std::in_place, 20);
    e.emplace(std::move(arg), 2);
    // constructed (tmp), move_assigned (this), destructed (tmp)
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 2 + 2);
    ASSERT_EQ(arg.x, -1);
    Val::s = State::constructed;
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  {
    Arg arg(3);
    expected<Val_throw, Err> e(std::in_place, 30);
    bool did_throw = false;
    try {
      Val_throw::t = May_throw::do_throw;
      e.emplace(std::move(arg), 3);
    } catch (...) {
      ASSERT_EQ(Val_throw::s, State::constructed); // failed
      ASSERT_EQ(Err::s, State::none);
      did_throw = true;
      Val_throw::t = May_throw::do_not_throw;
    }
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 30);
    ASSERT_EQ(arg.x, -1);
    ASSERT_TRUE(did_throw);
  }
  ASSERT_EQ(Val_throw::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val_throw::reset();
  // !has_value() via std::is_nothrow_constructible_v<T, Args...>
  {
    Arg arg(5);
    expected<Val, Err> e(unexpect, 50);
    e.emplace(std::move(arg), 5);
    ASSERT_EQ(Val::s, State::constructed);
    ASSERT_EQ(Err::s, State::destructed);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 5 + 5);
    ASSERT_EQ(arg.x, -1);
    Err::reset();
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  // !has_value() via std::is_nothrow_move_constructible_v<T>
  {
    Arg arg(7);
    expected<Val_throw, Err> e(unexpect, 70);
    e.emplace(std::move(arg), 7);
    // constructed (tmp), move_constructed (this), destructed (tmp)
    ASSERT_EQ(Val_throw::s, State::destructed);
    ASSERT_EQ(Err::s, State::destructed);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 7 + 7);
    ASSERT_EQ(arg.x, -1);
    Val_throw::s = State::constructed;
    Err::reset();
  }
  ASSERT_EQ(Val_throw::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val_throw::reset();
  {
    Arg arg(8);
    expected<Val_throw, Err> e(unexpect, 80);
    bool did_throw = false;
    try {
      Val_throw::t = May_throw::do_throw;
      e.emplace(std::move(arg), 8);
    } catch (...) {
      ASSERT_EQ(Val_throw::s, State::constructed); // failed
      ASSERT_EQ(Err::s, State::constructed);
      did_throw = true;
      Val_throw::t = May_throw::do_not_throw;
    }
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 80);
    ASSERT_EQ(arg.x, -1);
    ASSERT_TRUE(did_throw);
    Val_throw::reset();
  }
  ASSERT_EQ(Val_throw::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  // !has_value() via std::is_nothrow_move_constructible_v<E>
  {
    Arg arg(10);
    expected<Val_throw_2, Err> e(unexpect, 100);
    e.emplace(std::move(arg), 10);
    ASSERT_EQ(Val_throw_2::s, State::constructed);
    ASSERT_EQ(Err::s, State::destructed);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 10 + 10);
    ASSERT_EQ(arg.x, -1);
    Err::reset();
  }
  ASSERT_EQ(Val_throw_2::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val_throw_2::reset();
  {
    Arg arg(11);
    expected<Val_throw_2, Err> e(unexpect, 110);
    bool did_throw = false;
    try {
      Val_throw_2::t = May_throw::do_throw;
      e.emplace(std::move(arg), 11);
    } catch (...) {
      ASSERT_EQ(Val_throw_2::s, State::constructed); // failed
      // move_constructed (tmp), move_constructed (this), destructed (tmp)
      ASSERT_EQ(Err::s, State::destructed);
      did_throw = true;
      Val_throw_2::t = May_throw::do_not_throw;
    }
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 110);
    ASSERT_EQ(arg.x, -1);
    ASSERT_TRUE(did_throw);
    Val_throw_2::reset();
    Err::s = State::constructed;
  }
  ASSERT_EQ(Val_throw_2::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
}

TEST(expected, emplace_initializer_list_overload) {
  Val::reset();
  Err::reset();
  Val_throw::reset();
  Val_throw_2::reset();
  // has_value()
  {
    Arg arg(2);
    expected<Val, Err> e(std::in_place, 20);
    e.emplace({2}, std::move(arg), 2);
    // constructed (tmp), move_assigned (this), destructed (tmp)
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 2 + 2 + 2);
    ASSERT_EQ(arg.x, -1);
    Val::s = State::constructed;
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  {
    Arg arg(3);
    expected<Val_throw, Err> e(std::in_place, 30);
    bool did_throw = false;
    try {
      Val_throw::t = May_throw::do_throw;
      e.emplace({3}, std::move(arg), 3);
    } catch (...) {
      ASSERT_EQ(Val_throw::s, State::constructed); // failed
      ASSERT_EQ(Err::s, State::none);
      did_throw = true;
      Val_throw::t = May_throw::do_not_throw;
    }
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 30);
    ASSERT_EQ(arg.x, -1);
    ASSERT_TRUE(did_throw);
  }
  ASSERT_EQ(Val_throw::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val_throw::reset();
  // !has_value() via std::is_nothrow_constructible_v<T, Args...>
  {
    Arg arg(5);
    expected<Val, Err> e(unexpect, 50);
    e.emplace({5}, std::move(arg), 5);
    ASSERT_EQ(Val::s, State::constructed);
    ASSERT_EQ(Err::s, State::destructed);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 5 + 5 + 5);
    ASSERT_EQ(arg.x, -1);
    Err::reset();
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  // !has_value() via std::is_nothrow_move_constructible_v<T>
  {
    Arg arg(7);
    expected<Val_throw, Err> e(unexpect, 70);
    e.emplace({7}, std::move(arg), 7);
    // constructed (tmp), move_constructed (this), destructed (tmp)
    ASSERT_EQ(Val_throw::s, State::destructed);
    ASSERT_EQ(Err::s, State::destructed);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 7 + 7 + 7);
    ASSERT_EQ(arg.x, -1);
    Val_throw::s = State::constructed;
    Err::reset();
  }
  ASSERT_EQ(Val_throw::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val_throw::reset();
  {
    Arg arg(8);
    expected<Val_throw, Err> e(unexpect, 80);
    bool did_throw = false;
    try {
      Val_throw::t = May_throw::do_throw;
      e.emplace({8}, std::move(arg), 8);
    } catch (...) {
      ASSERT_EQ(Val_throw::s, State::constructed); // failed
      ASSERT_EQ(Err::s, State::constructed);
      did_throw = true;
      Val_throw::t = May_throw::do_not_throw;
    }
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 80);
    ASSERT_EQ(arg.x, -1);
    ASSERT_TRUE(did_throw);
    Val_throw::reset();
  }
  ASSERT_EQ(Val_throw::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  // !has_value() via std::is_nothrow_move_constructible_v<E>
  {
    Arg arg(10);
    expected<Val_throw_2, Err> e(unexpect, 100);
    e.emplace({10}, std::move(arg), 10);
    ASSERT_EQ(Val_throw_2::s, State::constructed);
    ASSERT_EQ(Err::s, State::destructed);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 10 + 10 + 10);
    ASSERT_EQ(arg.x, -1);
    Err::reset();
  }
  ASSERT_EQ(Val_throw_2::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val_throw_2::reset();
  {
    Arg arg(11);
    expected<Val_throw_2, Err> e(unexpect, 110);
    bool did_throw = false;
    try {
      Val_throw_2::t = May_throw::do_throw;
      e.emplace({11}, std::move(arg), 11);
    } catch (...) {
      ASSERT_EQ(Val_throw_2::s, State::constructed); // failed
      // move_constructed (tmp), move_constructed (this), destructed (tmp)
      ASSERT_EQ(Err::s, State::destructed);
      did_throw = true;
      Val_throw_2::t = May_throw::do_not_throw;
    }
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 110);
    ASSERT_EQ(arg.x, -1);
    ASSERT_TRUE(did_throw);
    Val_throw_2::reset();
    Err::s = State::constructed;
  }
  ASSERT_EQ(Val_throw_2::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
}

TEST(expected, swap_traits) {
  ASSERT_TRUE((std::is_swappable_v<expected<Val, Err>>));
  ASSERT_TRUE((std::is_nothrow_swappable_v<expected<Val, Err>>));

  ASSERT_TRUE((std::is_swappable_v<expected<Val_throw_2, Err>>));
  ASSERT_FALSE((std::is_nothrow_swappable_v<expected<Val_throw_2, Err>>));

  ASSERT_TRUE((std::is_swappable_v<expected<Val, Err_throw_2>>));
  ASSERT_FALSE((std::is_nothrow_swappable_v<expected<Val, Err_throw_2>>));

  ASSERT_FALSE((std::is_swappable_v<expected<Val_throw_2, Err_throw_2>>));
}

TEST(expected, swap) {
  // this->has_value() && other.has_value()
  {
    expected<Val, Err> other(std::in_place, 1);
    {
      expected<Val, Err> e(std::in_place, 10);
      bc::exp::swap(e, other);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
      ASSERT_EQ(e->x, 1);
      ASSERT_EQ(other->x, 10);
      Val::reset();
      Err::reset();
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  // this->has_value() && !other.has_value() via
  // std::is_nothrow_move_constructible_v<E>
  {
    expected<Val_throw_2, Err> other(unexpect, 2);
    {
      expected<Val_throw_2, Err> e(std::in_place, 20);
      bc::exp::swap(e, other);
      ASSERT_FALSE(e.has_value());
      ASSERT_TRUE(other.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(other->x, 20);
      Val_throw_2::reset();
      Err::reset();
    }
    ASSERT_EQ(Val_throw_2::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  ASSERT_EQ(Val_throw_2::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val_throw_2::reset();
  // this->has_value() && !other.has_value() via
  // std::is_nothrow_move_constructible_v<T>
  {
    expected<Val, Err_throw_2> other(unexpect, 3);
    {
      expected<Val, Err_throw_2> e(std::in_place, 30);
      bc::exp::swap(e, other);
      ASSERT_FALSE(e.has_value());
      ASSERT_TRUE(other.has_value());
      ASSERT_EQ(e.error().x, 3);
      ASSERT_EQ(other->x, 30);
      Val::reset();
      Err_throw_2::reset();
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_throw_2::s, State::destructed);
    Err_throw_2::reset();
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err_throw_2::s, State::none);
  Val::reset();
  // !this->has_value() && other.has_value()
  // std::is_nothrow_move_constructible_v<E>
  {
    expected<Val_throw_2, Err> other(std::in_place, 4);
    {
      expected<Val_throw_2, Err> e(unexpect, 40);
      bc::exp::swap(e, other);
      ASSERT_TRUE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e->x, 4);
      ASSERT_EQ(other.error().x, 40);
      Val_throw_2::reset();
      Err::reset();
    }
    ASSERT_EQ(Val_throw_2::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val_throw_2::reset();
  }
  ASSERT_EQ(Val_throw_2::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  // !this->has_value() && other.has_value()
  // std::is_nothrow_move_constructible_v<T>
  {
    expected<Val, Err_throw_2> other(std::in_place, 5);
    {
      expected<Val, Err_throw_2> e(unexpect, 50);
      bc::exp::swap(e, other);
      ASSERT_TRUE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e->x, 5);
      ASSERT_EQ(other.error().x, 50);
      Val::reset();
      Err_throw_2::reset();
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err_throw_2::s, State::none);
    Val::reset();
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err_throw_2::s, State::destructed);
  Err_throw_2::reset();
  // !this->has_value() && !other.has_value()
  {
    expected<Val, Err> other(unexpect, 6);
    {
      expected<Val, Err> e(unexpect, 60);
      bc::exp::swap(e, other);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 6);
      ASSERT_EQ(other.error().x, 60);
      Val::reset();
      Err::reset();
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
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
