#include "bc/expected.h"

#include "obj.h"
#include "obj_trivial.h"
#include "state.h"
#include "union_access.h"

#include <type_traits>

#include <gtest/gtest.h>

using namespace bc;

namespace {

using Base = detail::expected_copy_base<Val, Err>;
using B_e_trivial = detail::expected_copy_base<Val, Err_trivial>;
using B_t_trivial = detail::expected_copy_base<Val_trivial, Err>;
using B_trivial = detail::expected_copy_base<Val_trivial, Err_trivial>;

using Base_void = detail::expected_copy_base<void, Err>;
using B_void_trivial = detail::expected_copy_base<void, Err_trivial>;

} // namespace

TEST(expected_copy_base, type_traits) {
  ASSERT_FALSE(std::is_trivially_copy_constructible_v<Val>);
  ASSERT_FALSE(std::is_trivially_copy_constructible_v<Err>);
  ASSERT_TRUE(std::is_trivially_copy_constructible_v<Val_trivial>);
  ASSERT_TRUE(std::is_trivially_copy_constructible_v<Err_trivial>);

  ASSERT_FALSE(std::is_trivially_copy_constructible_v<Base>);
  ASSERT_FALSE(std::is_trivially_copy_constructible_v<B_e_trivial>);
  ASSERT_FALSE(std::is_trivially_copy_constructible_v<B_t_trivial>);
  ASSERT_TRUE(std::is_trivially_copy_constructible_v<B_trivial>);

  ASSERT_FALSE(std::is_trivially_copy_constructible_v<Base_void>);
  ASSERT_TRUE(std::is_trivially_copy_constructible_v<B_void_trivial>);
}

TEST(expected_copy_base, copy_constructor) {
  Val::reset();
  Err::reset();
  // other.has_value()
  {
    Base other(std::in_place, 1);
    Val::reset();
    {
      Base b(other);
      ASSERT_EQ(Val::s, State::copy_constructed);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(has_val(b));
      ASSERT_TRUE(has_val(other));
      ASSERT_EQ(val(b).x, 1);
      ASSERT_EQ(val(other).x, 1);
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  // !other.has_value()
  {
    Base other(unexpect, 2);
    Err::reset();
    {
      Base b(other);
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::copy_constructed);
      ASSERT_FALSE(has_val(b));
      ASSERT_FALSE(has_val(other));
      ASSERT_EQ(err(b).x, 2);
      ASSERT_EQ(err(other).x, 2);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
}

TEST(expected_copy_base, copy_constructor_void) {
  Err::reset();
  // other.has_value()
  {
    Base_void other(std::in_place);
    {
      Base_void b(other);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(has_val(b));
      ASSERT_TRUE(has_val(other));
      (void)dummy(b);
      (void)dummy(other);
    }
    ASSERT_EQ(Err::s, State::none);
  }
  // !other.has_value()
  {
    Base_void other(unexpect, 1);
    Err::reset();
    {
      Base_void b(other);
      ASSERT_EQ(Err::s, State::copy_constructed);
      ASSERT_FALSE(has_val(b));
      ASSERT_FALSE(has_val(other));
      ASSERT_EQ(err(b).x, 1);
      ASSERT_EQ(err(other).x, 1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
}
