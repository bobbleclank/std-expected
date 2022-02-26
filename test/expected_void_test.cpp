#include "bc/exp/expected.h"

#include "arg.h"
#include "obj.h"
#include "obj_implicit.h"
#include "obj_throw.h"
#include "state.h"

#include <utility>

#include <gtest/gtest.h>

using namespace bc::exp;

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

TEST(expected_void, copy_unexpected_constructor) {
  Err::reset();
  Err_implicit::reset();
  // explicit with G != E
  {
    unexpected<Arg> val(1);
    {
      expected<void, Err> e(val);
      ASSERT_EQ(Err::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(val.value().x, 1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  // implicit with G != E
  {
    unexpected<Arg> val(2);
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
  Err::reset();
  Err_implicit::reset();
  // explicit with G != E
  {
    unexpected<Arg> val(1);
    {
      expected<void, Err> e(std::move(val));
      ASSERT_EQ(Err::s, State::constructed);
      ASSERT_FALSE(e.has_value());
      ASSERT_EQ(e.error().x, 1);
      ASSERT_EQ(val.value().x, -1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  // implicit with G != E
  {
    unexpected<Arg> val(2);
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
    ASSERT_EQ(e.error().x, 1);
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
    ASSERT_EQ(e.error().x, 2 + 2);
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
    unexpected<Arg> val(1);
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
    unexpected<Arg> val(2);
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
    unexpected<Arg> val(3);
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
    unexpected<Arg> val(1);
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
    unexpected<Arg> val(2);
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
    unexpected<Arg> val(3);
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
