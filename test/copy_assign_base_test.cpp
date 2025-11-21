#include "bc/expected.h"

#include "obj.h"
#include "obj_trivial.h"
#include "state.h"
#include "union_access.h"

#include <type_traits>

#include <gtest/gtest.h>

using namespace bc;

namespace {

using Base = detail::expected_copy_assign_base<Val, Err>;
using B_e_trivial = detail::expected_copy_assign_base<Val, Err_trivial>;
using B_t_trivial = detail::expected_copy_assign_base<Val_trivial, Err>;
using B_trivial = detail::expected_copy_assign_base<Val_trivial, Err_trivial>;

using Base_void = detail::expected_copy_assign_base<void, Err>;
using B_void_trivial = detail::expected_copy_assign_base<void, Err_trivial>;

} // namespace

// NOLINTBEGIN(*-avoid-magic-numbers): Test values

TEST(expected_copy_assign_base, type_traits) {
  ASSERT_FALSE(std::is_trivially_copy_assignable_v<Val>);
  ASSERT_FALSE(std::is_trivially_copy_assignable_v<Err>);
  ASSERT_TRUE(std::is_trivially_copy_assignable_v<Val_trivial>);
  ASSERT_TRUE(std::is_trivially_copy_assignable_v<Err_trivial>);

  ASSERT_FALSE(std::is_trivially_copy_assignable_v<Base>);
  ASSERT_FALSE(std::is_trivially_copy_assignable_v<B_e_trivial>);
  ASSERT_FALSE(std::is_trivially_copy_assignable_v<B_t_trivial>);
  ASSERT_TRUE(std::is_trivially_copy_assignable_v<B_trivial>);

  ASSERT_FALSE(std::is_trivially_copy_assignable_v<Base_void>);
  ASSERT_TRUE(std::is_trivially_copy_assignable_v<B_void_trivial>);
}

TEST(expected_copy_assign_base, copy_assignment_operator) {
  Val::reset();
  Err::reset();
  // this->has_value() && other.has_value()
  {
    Base other(std::in_place, 1);
    Val::reset();
    {
      Base b(std::in_place, 10);
      b = other;
      ASSERT_EQ(Val::s, State::copy_assigned);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(val(b).x, 1);
      ASSERT_EQ(val(other).x, 1);
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  // !this->has_value() && !other.has_value()
  {
    Base other(unexpect, 2);
    Err::reset();
    {
      Base b(unexpect, 20);
      b = other;
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::copy_assigned);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(unexpect_value(b).x, 2);
      ASSERT_EQ(unexpect_value(other).x, 2);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
}

TEST(expected_copy_assign_base, copy_assignment_operator_void) {
  Err::reset();
  // this->has_value() && other.has_value()
  {
    Base_void other(std::in_place);
    {
      Base_void b(std::in_place);
      b = other;
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      (void)dummy(b);
      (void)dummy(other);
    }
    ASSERT_EQ(Err::s, State::none);
  }
  // !this->has_value() && !other.has_value()
  {
    Base_void other(unexpect, 1);
    Err::reset();
    {
      Base_void b(unexpect, 10);
      b = other;
      ASSERT_EQ(Err::s, State::copy_assigned);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(unexpect_value(b).x, 1);
      ASSERT_EQ(unexpect_value(other).x, 1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
}

// NOLINTEND(*-avoid-magic-numbers): Test values
