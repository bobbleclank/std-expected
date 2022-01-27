#include "bc/exp/expected.h"

#include "arg.h"
#include "obj.h"

#include <utility>

#include <gtest/gtest.h>

using namespace bc::exp;

TEST(unexpected, value) {
  // const& overload
  {
    const unexpected<Err> e(1);
    const Err& err = e.value();
    ASSERT_EQ(err.x, 1);
    ASSERT_EQ(e.value().x, 1);
  }
  // non-const& overload
  {
    unexpected<Err> e(2);
    Err& err = e.value();
    ASSERT_EQ(err.x, 2);
    ASSERT_EQ(e.value().x, 2);
    err.x = 20;
    ASSERT_EQ(err.x, 20);
    ASSERT_EQ(e.value().x, 20);
  }
  // const&& overload
  {
    const unexpected<Err> e(3);
    Err err = std::move(e).value();
    ASSERT_EQ(err.x, 3);
    ASSERT_EQ(e.value().x, 3);
    // Since the r-value reference is const, Err's copy constructor is called,
    // and not Err's move constructor (which takes a non-const r-value
    // reference).
  }
  {
    const unexpected<Err> e(4);
    Err err;
    err = std::move(e).value();
    ASSERT_EQ(err.x, 4);
    ASSERT_EQ(e.value().x, 4);
    // Since the r-value reference is const, Err's copy assignment is called,
    // and not Err's move assignment (which takes a non-const r-value
    // reference).
  }
  // non-const&& overload
  {
    unexpected<Err> e(5);
    Err err = std::move(e).value();
    ASSERT_EQ(err.x, 5);
    ASSERT_EQ(e.value().x, -1);
  }
  {
    unexpected<Err> e(6);
    Err err;
    err = std::move(e).value();
    ASSERT_EQ(err.x, 6);
    ASSERT_EQ(e.value().x, -2);
  }
}

TEST(unexpected, constructors) {
  // Deduction guide
  {
    Err val(1);
    unexpected e(val);
    ASSERT_EQ(e.value().x, 1);
    ASSERT_EQ(val.x, 1);
  }
  // (const unexpected&)
  {
    unexpected<Err> other(2);
    unexpected<Err> e(other);
    ASSERT_EQ(e.value().x, 2);
    ASSERT_EQ(other.value().x, 2);
  }
  // (unexpected&&)
  {
    unexpected<Err> other(4);
    unexpected<Err> e(std::move(other));
    ASSERT_EQ(e.value().x, 4);
    ASSERT_EQ(other.value().x, -1);
  }
  // (Err&&) with Err = E
  {
    Err val(3);
    unexpected<Err> e(std::move(val));
    ASSERT_EQ(e.value().x, 3);
    ASSERT_EQ(val.x, -1);
  }
  // (Err&&) with Err != E
  {
    Arg val(6);
    unexpected<Err> e(std::move(val));
    ASSERT_EQ(e.value().x, 6);
    ASSERT_EQ(val.x, -1);
  }
  // (std::in_place_t, Args&&...)
  {
    Arg arg(7);
    unexpected<Err> e(std::in_place, std::move(arg), 7);
    ASSERT_EQ(e.value().x, 7);
    ASSERT_EQ(arg.x, -1);
  }
  // (std::in_place_t, std::initializer_list<U>, Args&&...)
  {
    Arg arg(8);
    unexpected<Err> e(std::in_place, {8}, std::move(arg), 8);
    ASSERT_EQ(e.value().x, 8 + 8);
    ASSERT_EQ(arg.x, -1);
  }
}

TEST(unexpected, assignment_operators) {
  // (const unexpected&)
  {
    unexpected<Err> other(1);
    unexpected<Err> e(10);
    e = other;
    ASSERT_EQ(e.value().x, 1);
    ASSERT_EQ(other.value().x, 1);
  }
  // (unexpected&&)
  {
    unexpected<Err> other(2);
    unexpected<Err> e(20);
    e = std::move(other);
    ASSERT_EQ(e.value().x, 2);
    ASSERT_EQ(other.value().x, -2);
  }
}

TEST(unexpected, equality_operators) {
  unexpected<Err> e_one(1);

  unexpected<Err> e1(1);
  unexpected<Err> e2(2);

  unexpected<Err2> e_two(2);

  // Operands have same type.

  ASSERT_TRUE(e_one == e1);
  ASSERT_FALSE(e_one == e2);
  ASSERT_FALSE(e_one != e1);
  ASSERT_TRUE(e_one != e2);

  // Operands have different types.

  ASSERT_FALSE(e_two == e1);
  ASSERT_TRUE(e_two == e2);
  ASSERT_TRUE(e_two != e1);
  ASSERT_FALSE(e_two != e2);
}
