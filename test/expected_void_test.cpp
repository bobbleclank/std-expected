#include "bc/expected.h"

#include "arg.h"
#include "obj.h"
#include "obj_implicit.h"
#include "obj_throw.h"
#include "state.h"

#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

using namespace bc;

// NOLINTBEGIN(*-avoid-magic-numbers): Test values

TEST(expected_void, error) {
  // const& overload
  {
    const expected<void, Err> e(unexpect, 1);
    const Err& err = e.error();
    ASSERT_EQ(err.x, 1);
    ASSERT_EQ(e.error().x, 1);
  }
  // non-const& overload
  {
    expected<void, Err> e(unexpect, 2);
    Err& err = e.error();
    ASSERT_EQ(err.x, 2);
    ASSERT_EQ(e.error().x, 2);
  }
  // const&& overload
  {
    const expected<void, Err> e(unexpect, 3);
    Err err = std::move(e).error();
    ASSERT_EQ(err.x, 3);
    ASSERT_EQ(e.error().x, 3);
  }
  // non-const&& overload
  {
    expected<void, Err> e(unexpect, 4);
    Err err = std::move(e).error();
    ASSERT_EQ(err.x, 4);
    ASSERT_EQ(e.error().x, -1);
  }
}

TEST(expected_void, value) {
  {
    expected<void, Err> e(std::in_place);
    e.value();
  }
  {
    expected<void, Err> e(unexpect, 1);
    bool did_throw = false;
    try {
      e.value();
    } catch (const bad_expected_access<Err>& ex) {
      ASSERT_EQ(ex.error().x, 1);
      did_throw = true;
    }
    ASSERT_EQ(e.error().x, 1);
    ASSERT_TRUE(did_throw);
  }
}

TEST(expected_void, has_value) {
  {
    expected<void, Err> e;
    ASSERT_TRUE(static_cast<bool>(e));
    ASSERT_TRUE(e.has_value());
  }
  {
    expected<void, Err> e(std::in_place);
    ASSERT_TRUE(static_cast<bool>(e));
    ASSERT_TRUE(e.has_value());
  }
  {
    expected<void, Err> e(unexpect);
    ASSERT_FALSE(static_cast<bool>(e));
    ASSERT_FALSE(e.has_value());
  }
}

TEST(expected_void, default_constructor) {
  Err::reset();
  {
    expected<void, Err> e;
    ASSERT_EQ(Err::s, State::none);
    ASSERT_TRUE(e.has_value());
  }
  ASSERT_EQ(Err::s, State::none);
}

TEST(expected_void, copy_constructor) {
  Err::reset();
  {
    expected<void, Err> other(std::in_place);
    {
      expected<void, Err> e(other);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
    }
    ASSERT_EQ(Err::s, State::none);
  }
  {
    expected<void, Err> other(unexpect, 1);
    Err::reset();
    {
      expected<void, Err> e(other);
      ASSERT_EQ(Err::s, State::copy_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(other.error().x, 1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
}

TEST(expected_void, move_constructor) {
  Err::reset();
  {
    expected<void, Err> other(std::in_place);
    {
      expected<void, Err> e(std::move(other));
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
    }
    ASSERT_EQ(Err::s, State::none);
  }
  {
    expected<void, Err> other(unexpect, 1);
    Err::reset();
    {
      expected<void, Err> e(std::move(other));
      ASSERT_EQ(Err::s, State::move_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(other.error().x, -1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
}

TEST(expected_void, copy_expected_constructor) {
  Err::reset();
  Err_implicit::reset();
  // explicit
  {
    expected<void, Arg> other(std::in_place);
    {
      expected<void, Err> e(other);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
    }
    ASSERT_EQ(Err::s, State::none);
  }
  {
    expected<void, Arg> other(unexpect, 1);
    {
      expected<void, Err> e(other);
      ASSERT_EQ(Err::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(other.error().x, 1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  // implicit
  {
    expected<void, Arg> other(std::in_place);
    {
      expected<void, Err_implicit> e = other;
      ASSERT_EQ(Err_implicit::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
    }
    ASSERT_EQ(Err_implicit::s, State::none);
  }
  {
    expected<void, Arg> other(unexpect, 2);
    {
      expected<void, Err_implicit> e = other;
      ASSERT_EQ(Err_implicit::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(other.error().x, 2);
    }
    ASSERT_EQ(Err_implicit::s, State::destructed);
    Err_implicit::reset();
  }
}

TEST(expected_void, move_expected_constructor) {
  Err::reset();
  Err_implicit::reset();
  // explicit
  {
    expected<void, Arg> other(std::in_place);
    {
      expected<void, Err> e(std::move(other));
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
    }
    ASSERT_EQ(Err::s, State::none);
  }
  {
    expected<void, Arg> other(unexpect, 1);
    {
      expected<void, Err> e(std::move(other));
      ASSERT_EQ(Err::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(other.error().x, -1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  // implicit
  {
    expected<void, Arg> other(std::in_place);
    {
      expected<void, Err_implicit> e = std::move(other);
      ASSERT_EQ(Err_implicit::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
    }
    ASSERT_EQ(Err_implicit::s, State::none);
  }
  {
    expected<void, Arg> other(unexpect, 2);
    {
      expected<void, Err_implicit> e = std::move(other);
      ASSERT_EQ(Err_implicit::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(other.error().x, -1);
    }
    ASSERT_EQ(Err_implicit::s, State::destructed);
    Err_implicit::reset();
  }
}

TEST(expected_void, copy_unexpected_constructor) {
  Err_implicit::reset();
  Err_explicit::reset();
  // explicit with G != E
  {
    unexpected val(Arg(1));
    {
      expected<void, Err_explicit> e(val);
      ASSERT_EQ(Err_explicit::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(val.value().x, 1);
    }
    ASSERT_EQ(Err_explicit::s, State::destructed);
    Err_explicit::reset();
  }
  // implicit with G != E
  {
    unexpected val(Arg(2));
    {
      expected<void, Err_implicit> e = val;
      ASSERT_EQ(Err_implicit::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(val.value().x, 2);
    }
    ASSERT_EQ(Err_implicit::s, State::destructed);
    Err_implicit::reset();
  }
}

TEST(expected_void, move_unexpected_constructor) {
  Err_implicit::reset();
  Err_explicit::reset();
  // explicit with G != E
  {
    unexpected val(Arg(1));
    {
      expected<void, Err_explicit> e(std::move(val));
      ASSERT_EQ(Err_explicit::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(val.value().x, -1);
    }
    ASSERT_EQ(Err_explicit::s, State::destructed);
    Err_explicit::reset();
  }
  // implicit with G != E
  {
    unexpected val(Arg(2));
    {
      expected<void, Err_implicit> e = std::move(val);
      ASSERT_EQ(Err_implicit::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(val.value().x, -1);
    }
    ASSERT_EQ(Err_implicit::s, State::destructed);
    Err_implicit::reset();
  }
}

TEST(expected_void, in_place_constructor) {
  Err::reset();
  // (std::in_place_t, Args&&...)
  {
    expected<void, Err> e(std::in_place);
    ASSERT_EQ(Err::s, State::none);
    ASSERT_TRUE(e.has_value());
  }
  // (unexpect_t, Args&&...)
  {
    expected<void, Err> e(unexpect);
    ASSERT_EQ(Err::s, State::default_constructed);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 20100);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(1);
    expected<void, Err> e(unexpect, std::move(arg), 1);
    ASSERT_EQ(Err::s, State::constructed);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 1 + 1);
    ASSERT_EQ(arg.x, -1);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  // (unexpect_t, std::initializer_list<U>, Args&&...)
  {
    Arg arg(2);
    expected<void, Err> e(unexpect, {2}, std::move(arg), 2);
    ASSERT_EQ(Err::s, State::constructed);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 2 + 2 + 2);
    ASSERT_EQ(arg.x, -1);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
}

TEST(expected_void, copy_assignment_operator) {
  Err::reset();
  {
    expected<void, Err> other(std::in_place);
    {
      expected<void, Err> e(std::in_place);
      e = other;
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
    }
    ASSERT_EQ(Err::s, State::none);
  }
  {
    expected<void, Err> other(std::in_place);
    {
      expected<void, Err> e(unexpect, 10);
      e = other;
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
      Err::reset();
    }
    ASSERT_EQ(Err::s, State::none);
  }
  {
    expected<void, Err> other(unexpect, 2);
    Err::reset();
    {
      expected<void, Err> e(unexpect, 20);
      e = other;
      ASSERT_EQ(Err::s, State::copy_assigned);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(other.error().x, 2);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  {
    expected<void, Err> other(unexpect, 3);
    Err::reset();
    {
      expected<void, Err> e(std::in_place);
      e = other;
      ASSERT_EQ(Err::s, State::copy_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 3);
      ASSERT_EQ(other.error().x, 3);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
}

TEST(expected_void, move_assignment_operator) {
  Err::reset();
  {
    expected<void, Err> other(std::in_place);
    {
      expected<void, Err> e(std::in_place);
      e = std::move(other);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
    }
    ASSERT_EQ(Err::s, State::none);
  }
  {
    expected<void, Err> other(std::in_place);
    {
      expected<void, Err> e(unexpect, 10);
      e = std::move(other);
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
      Err::reset();
    }
    ASSERT_EQ(Err::s, State::none);
  }
  {
    expected<void, Err> other(unexpect, 2);
    Err::reset();
    {
      expected<void, Err> e(unexpect, 20);
      e = std::move(other);
      ASSERT_EQ(Err::s, State::move_assigned);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(other.error().x, -2);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  {
    expected<void, Err> other(unexpect, 3);
    Err::reset();
    {
      expected<void, Err> e(std::in_place);
      e = std::move(other);
      ASSERT_EQ(Err::s, State::move_constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 3);
      ASSERT_EQ(other.error().x, -1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
}

TEST(expected_void, copy_unexpected_assignment_operator) {
  Err::reset();
  Err_throw_3::reset();
  // has_value() with G != E
  {
    unexpected val(Arg(1));
    {
      expected<void, Err> e(std::in_place);
      e = val;
      ASSERT_EQ(Err::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(val.value().x, 1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  // !has_value() with G != E
  {
    unexpected val(Arg(2));
    {
      expected<void, Err> e(unexpect, 20);
      e = val;
      ASSERT_EQ(Err::s, State::assigned);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(val.value().x, 2);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  {
    unexpected val(Arg(3));
    {
      expected<void, Err_throw_3> e(unexpect, 30);
      bool did_throw = false;
      try {
        Err_throw_3::t = May_throw::do_throw;
        e = val;
      } catch (...) {
        ASSERT_EQ(Err_throw_3::s, State::assigned); // failed
        did_throw = true;
        Err_throw_3::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 3); // No exception safety guarantee.
      ASSERT_EQ(val.value().x, 3);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Err_throw_3::s, State::destructed);
    Err_throw_3::reset();
  }
}

TEST(expected_void, move_unexpected_assignment_operator) {
  Err::reset();
  Err_throw_3::reset();
  // has_value() with G != E
  {
    unexpected val(Arg(1));
    {
      expected<void, Err> e(std::in_place);
      e = std::move(val);
      ASSERT_EQ(Err::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(val.value().x, -1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  // !has_value() with G != E
  {
    unexpected val(Arg(2));
    {
      expected<void, Err> e(unexpect, 20);
      e = std::move(val);
      ASSERT_EQ(Err::s, State::assigned);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 2);
      ASSERT_EQ(val.value().x, -1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  {
    unexpected val(Arg(3));
    {
      expected<void, Err_throw_3> e(unexpect, 30);
      bool did_throw = false;
      try {
        Err_throw_3::t = May_throw::do_throw;
        e = std::move(val);
      } catch (...) {
        ASSERT_EQ(Err_throw_3::s, State::assigned); // failed
        did_throw = true;
        Err_throw_3::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 3); // No exception safety guarantee.
      ASSERT_EQ(val.value().x, -1);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Err_throw_3::s, State::destructed);
    Err_throw_3::reset();
  }
}

TEST(expected_void, emplace) {
  Err::reset();
  // has_value()
  {
    expected<void, Err> e(std::in_place);
    e.emplace();
    ASSERT_EQ(Err::s, State::none);
    ASSERT_TRUE(e.has_value());
  }
  ASSERT_EQ(Err::s, State::none);
  // !has_value()
  {
    expected<void, Err> e(unexpect, 10);
    e.emplace();
    ASSERT_EQ(Err::s, State::destructed);
    ASSERT_TRUE(e.has_value());
    Err::reset();
  }
  ASSERT_EQ(Err::s, State::none);
}

TEST(expected_void, swap_traits) {
  ASSERT_TRUE((std::is_swappable_v<expected<void, Err>>));
  ASSERT_TRUE((std::is_nothrow_swappable_v<expected<void, Err>>));

  ASSERT_TRUE((std::is_swappable_v<expected<void, Err_throw_2>>));
  ASSERT_FALSE((std::is_nothrow_swappable_v<expected<void, Err_throw_2>>));
}

TEST(expected_void, swap) {
  // this->has_value() && other.has_value()
  {
    expected<void, Err> other(std::in_place);
    {
      expected<void, Err> e(std::in_place);
      bc::swap(e, other);
      ASSERT_TRUE(e.has_value());
      ASSERT_TRUE(other.has_value());
      Err::reset();
    }
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::none);
  // this->has_value() && !other.has_value()
  {
    expected<void, Err> other(unexpect, 1);
    {
      expected<void, Err> e(std::in_place);
      bc::swap(e, other);
      ASSERT_FALSE(e.has_value());
      ASSERT_TRUE(other.has_value());
      ASSERT_EQ(e.error().x, 1);
      Err::reset();
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  ASSERT_EQ(Err::s, State::none);
  // !this->has_value() && other.has_value()
  {
    expected<void, Err> other(std::in_place);
    {
      expected<void, Err> e(unexpect, 10);
      bc::swap(e, other);
      ASSERT_TRUE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(other.error().x, 10);
      Err::reset();
    }
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  // !this->has_value() && !other.has_value()
  {
    expected<void, Err> other(unexpect, 3);
    {
      expected<void, Err> e(unexpect, 30);
      bc::swap(e, other);
      ASSERT_FALSE(e.has_value());
      ASSERT_FALSE(other.has_value());
      ASSERT_EQ(e.error().x, 3);
      ASSERT_EQ(other.error().x, 30);
      Err::reset();
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
}

TEST(expected_void, equality_operators) {
  expected<void, Err> e_one(std::in_place);
  expected<void, Err> u_one(unexpect, 1);

  expected<void, Err> e1(std::in_place);

  expected<void, Err> u1(unexpect, 1);
  expected<void, Err> u2(unexpect, 2);

  expected<void, Err2> e_two(std::in_place);
  expected<void, Err2> u_two(unexpect, 2);

  // Operands have same type.

  ASSERT_TRUE(e_one == e1);
  ASSERT_FALSE(e_one != e1);

  ASSERT_TRUE(u_one == u1);
  ASSERT_FALSE(u_one == u2);
  ASSERT_FALSE(u_one != u1);
  ASSERT_TRUE(u_one != u2);

  ASSERT_FALSE(e_one == u1);
  ASSERT_TRUE(e_one != u1);

  ASSERT_FALSE(u_one == e1);
  ASSERT_TRUE(u_one != e1);

  // Operands have different types.

  ASSERT_TRUE(e_two == e1);
  ASSERT_FALSE(e_two != e1);

  ASSERT_FALSE(u_two == u1);
  ASSERT_TRUE(u_two == u2);
  ASSERT_TRUE(u_two != u1);
  ASSERT_FALSE(u_two != u2);

  ASSERT_FALSE(e_two == u1);
  ASSERT_TRUE(e_two != u1);

  ASSERT_FALSE(u_two == e1);
  ASSERT_TRUE(u_two != e1);
}

TEST(expected_void, comparison_with_unexpected_E) {
  expected<void, Err> e_one(std::in_place);
  expected<void, Err> u_one(unexpect, 1);

  unexpected<Err> v1(1);
  unexpected<Err> v2(2);

  expected<void, Err2> e_two(std::in_place);
  expected<void, Err2> u_two(unexpect, 2);

  // expected::error_type and E have same type.

  ASSERT_TRUE(u_one == v1);
  ASSERT_FALSE(u_one == v2);
  ASSERT_FALSE(u_one != v1);
  ASSERT_TRUE(u_one != v2);

  ASSERT_FALSE(e_one == v1);
  ASSERT_TRUE(e_one != v1);

  ASSERT_TRUE(v1 == u_one);
  ASSERT_FALSE(v2 == u_one);
  ASSERT_FALSE(v1 != u_one);
  ASSERT_TRUE(v2 != u_one);

  ASSERT_FALSE(v1 == e_one);
  ASSERT_TRUE(v1 != e_one);

  // expected::error_type and E have different types.

  ASSERT_FALSE(u_two == v1);
  ASSERT_TRUE(u_two == v2);
  ASSERT_TRUE(u_two != v1);
  ASSERT_FALSE(u_two != v2);

  ASSERT_FALSE(e_two == v1);
  ASSERT_TRUE(e_two != v1);

  ASSERT_FALSE(v1 == u_two);
  ASSERT_TRUE(v2 == u_two);
  ASSERT_TRUE(v1 != u_two);
  ASSERT_FALSE(v2 != u_two);

  ASSERT_FALSE(v1 == e_two);
  ASSERT_TRUE(v1 != e_two);
}

// NOLINTEND(*-avoid-magic-numbers): Test values
