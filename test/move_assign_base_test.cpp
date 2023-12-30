#include "bc/expected.h"

#include "obj.h"
#include "obj_throw.h"
#include "obj_trivial.h"
#include "state.h"

#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

using namespace bc;
using namespace bc::detail;

namespace {

using Base = expected_move_assign_base<Val, Err>;
using B_e_trivial = expected_move_assign_base<Val, Err_trivial>;
using B_t_trivial = expected_move_assign_base<Val_trivial, Err>;
using B_trivial = expected_move_assign_base<Val_trivial, Err_trivial>;

using Base_void = expected_move_assign_base<void, Err>;
using B_void_trivial = expected_move_assign_base<void, Err_trivial>;

using B_e_throw_2 = expected_move_assign_base<Val, Err_throw_2>;
using B_t_throw_2 = expected_move_assign_base<Val_throw_2, Err>;
using B_throw_2 = expected_move_assign_base<Val_throw_2, Err_throw_2>;

using B_void_throw_2 = expected_move_assign_base<void, Err_throw_2>;

} // namespace

TEST(expected_move_assign_base, type_traits) {
  // is_trivially_move_assignable

  ASSERT_FALSE(std::is_trivially_move_assignable_v<Val>);
  ASSERT_FALSE(std::is_trivially_move_assignable_v<Err>);
  ASSERT_TRUE(std::is_trivially_move_assignable_v<Val_trivial>);
  ASSERT_TRUE(std::is_trivially_move_assignable_v<Err_trivial>);

  ASSERT_FALSE(std::is_trivially_move_assignable_v<Base>);
  ASSERT_FALSE(std::is_trivially_move_assignable_v<B_e_trivial>);
  ASSERT_FALSE(std::is_trivially_move_assignable_v<B_t_trivial>);
  ASSERT_TRUE(std::is_trivially_move_assignable_v<B_trivial>);

  ASSERT_FALSE(std::is_trivially_move_assignable_v<Base_void>);
  ASSERT_TRUE(std::is_trivially_move_assignable_v<B_void_trivial>);

  // is_nothrow_move_assignable

  ASSERT_TRUE(std::is_nothrow_move_assignable_v<Val>);
  ASSERT_TRUE(std::is_nothrow_move_assignable_v<Err>);
  ASSERT_FALSE(std::is_nothrow_move_assignable_v<Val_throw_2>);
  ASSERT_FALSE(std::is_nothrow_move_assignable_v<Err_throw_2>);

  ASSERT_TRUE(std::is_nothrow_move_assignable_v<Base>);
  ASSERT_FALSE(std::is_nothrow_move_assignable_v<B_e_throw_2>);
  ASSERT_FALSE(std::is_nothrow_move_assignable_v<B_t_throw_2>);
  ASSERT_FALSE(std::is_nothrow_move_assignable_v<B_throw_2>);

  ASSERT_TRUE(std::is_nothrow_move_assignable_v<Base_void>);
  ASSERT_FALSE(std::is_nothrow_move_assignable_v<B_void_throw_2>);
}

TEST(expected_move_assign_base, move_assignment_operator) {
  Val::reset();
  Err::reset();
  // this->has_value() && other.has_value()
  {
    Base other(std::in_place, 1);
    Val::reset();
    {
      Base b(std::in_place, 10);
      b = std::move(other);
      ASSERT_EQ(Val::s, State::move_assigned);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(b.val_.x, 1);
      ASSERT_EQ(other.val_.x, -2);
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
      b = std::move(other);
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::move_assigned);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 2);
      ASSERT_EQ(other.unexpect_.value().x, -2);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
}

TEST(expected_move_assign_base, move_assignment_operator_void) {
  Err::reset();
  // this->has_value() && other.has_value()
  {
    Base_void other(std::in_place);
    {
      Base_void b(std::in_place);
      b = std::move(other);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      (void)b.dummy_;
      (void)other.dummy_;
    }
    ASSERT_EQ(Err::s, State::none);
  }
  // !this->has_value() && !other.has_value()
  {
    Base_void other(unexpect, 1);
    Err::reset();
    {
      Base_void b(unexpect, 10);
      b = std::move(other);
      ASSERT_EQ(Err::s, State::move_assigned);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 1);
      ASSERT_EQ(other.unexpect_.value().x, -2);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
}
