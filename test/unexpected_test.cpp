#include "bc/expected.h"

#include "arg.h"
#include "obj.h"
#include "obj_implicit.h"
#include "obj_throw.h"

#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

using namespace bc;

// NOLINTBEGIN(*-avoid-magic-numbers): Test values

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
    Err err(40);
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
    Err err(60);
    err = std::move(e).value();
    ASSERT_EQ(err.x, 6);
    ASSERT_EQ(e.value().x, -2);
  }
}

TEST(unexpected, constructors) {
  // Deduction guide via (Err&&) with copy
  {
    Err val(1);
    unexpected e(val);
    ASSERT_TRUE(
        (std::is_same_v<std::remove_reference_t<decltype(e.value())>, Err>));
    ASSERT_EQ(e.value().x, 1);
    ASSERT_EQ(val.x, 1);
  }
  // Deduction guide via (Err&&) with move
  {
    Err val(12);
    unexpected e(std::move(val));
    ASSERT_TRUE(
        (std::is_same_v<std::remove_reference_t<decltype(e.value())>, Err>));
    ASSERT_EQ(e.value().x, 12);
    ASSERT_EQ(val.x, -1);
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
  // explicit (const unexpected<Err>&)
  {
    unexpected<Arg> other(5);
    unexpected<Err> e(other);
    ASSERT_EQ(e.value().x, 5);
    ASSERT_EQ(other.value().x, 5);
  }
  // implicit (const unexpected<Err>&)
  {
    unexpected<Arg> other(9);
    unexpected<Err_implicit> e = other;
    ASSERT_EQ(e.value().x, 9);
    ASSERT_EQ(other.value().x, 9);
  }
  // explicit (unexpected<Err>&&)
  {
    unexpected<Arg> other(10);
    unexpected<Err> e(std::move(other));
    ASSERT_EQ(e.value().x, 10);
    ASSERT_EQ(other.value().x, -1);
  }
  // implicit (unexpected<Err>&&)
  {
    unexpected<Arg> other(11);
    unexpected<Err_implicit> e = std::move(other);
    ASSERT_EQ(e.value().x, 11);
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
    ASSERT_EQ(e.value().x, 7 + 7);
    ASSERT_EQ(arg.x, -1);
  }
  // (std::in_place_t, std::initializer_list<U>, Args&&...)
  {
    Arg arg(8);
    unexpected<Err> e(std::in_place, {8}, std::move(arg), 8);
    ASSERT_EQ(e.value().x, 8 + 8 + 8);
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
  // (const unexpected<Err>&)
  {
    unexpected<Arg> other(3);
    unexpected<Err> e(30);
    e = other;
    ASSERT_EQ(e.value().x, 3);
    ASSERT_EQ(other.value().x, 3);
  }
  // (unexpected<Err>&&)
  {
    unexpected<Arg> other(4);
    unexpected<Err> e(40);
    e = std::move(other);
    ASSERT_EQ(e.value().x, 4);
    ASSERT_EQ(other.value().x, -1);
  }
}

TEST(unexpected, swap_traits) {
  ASSERT_TRUE(std::is_swappable_v<unexpected<Err>>);
  ASSERT_TRUE(std::is_nothrow_swappable_v<unexpected<Err>>);

  ASSERT_TRUE(std::is_swappable_v<unexpected<Err_throw_2>>);
  ASSERT_FALSE(std::is_nothrow_swappable_v<unexpected<Err_throw_2>>);
}

TEST(unexpected, swap) {
  {
    unexpected<Err> other(1);
    unexpected<Err> e(10);
    bc::swap(e, other);
    ASSERT_EQ(e.value().x, 1);
    ASSERT_EQ(other.value().x, 10);
  }
  {
    unexpected<Err_throw_2> other(2);
    unexpected<Err_throw_2> e(20);
    bool did_throw = false;
    try {
      Err_throw_2::t = May_throw::do_throw;
      bc::swap(e, other);
    } catch (...) {
      did_throw = true;
      Err_throw_2::t = May_throw::do_not_throw;
    }
    ASSERT_EQ(e.value().x, -1); // No exception safety guarantee.
    ASSERT_EQ(other.value().x, 2);
    ASSERT_TRUE(did_throw);
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

// NOLINTEND(*-avoid-magic-numbers): Test values
