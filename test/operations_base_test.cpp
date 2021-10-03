#include "exp/expected.h"

#include "arg.h"
#include "obj.h"
#include "state.h"

#include <utility>

#include <gtest/gtest.h>

using namespace exp::internal;

namespace {

using Base = expected_operations_base<Val, Err>;
using Base_void = expected_operations_base<void, Err>;

} // namespace

TEST(expected_operations_base, in_place_t_construct_destroy) {
  Val::reset();
  Err::reset();
  {
    Arg arg(1);
    Base b(uninit);
    b.construct(std::in_place, std::move(arg), 1);
    ASSERT_EQ(b.val_.x, 1);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::constructed);
    ASSERT_EQ(Err::s, State::none);
    b.destroy(std::in_place);
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  // T is void
  {
    Base_void b(uninit);
    b.construct(std::in_place);
    (void)b.dummy_;
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Err::s, State::none);
    b.destroy(std::in_place);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::none);
}

TEST(expected_operations_base, unexpect_t_construct_destroy) {
  Val::reset();
  Err::reset();
  {
    Arg arg(1);
    Base b(uninit);
    b.construct(exp::unexpect, std::in_place, std::move(arg), 1);
    ASSERT_EQ(b.unexpect_.value().x, 1);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::constructed);
    b.destroy(exp::unexpect);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  // T is void
  {
    Arg arg(2);
    Base_void b(uninit);
    b.construct(exp::unexpect, std::in_place, std::move(arg), 2);
    ASSERT_EQ(b.unexpect_.value().x, 2);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::constructed);
    b.destroy(exp::unexpect);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
}
