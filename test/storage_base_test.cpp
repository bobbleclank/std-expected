#include "exp/expected.h"

#include "arg.h"
#include "obj.h"
#include "obj_trivial.h"
#include "state.h"

#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

using namespace exp::internal;

namespace {

struct Trivial_t_tag {};
using Trivial_t = Obj_trivial<Trivial_t_tag>;

struct Trivial_e_tag {};
using Trivial_e = Obj_trivial<Trivial_e_tag>;

using Base_not_trivial = expected_storage_base<Val, Err>;
using Base_t_not_trivial = expected_storage_base<Val, Trivial_e>;
using Base_e_not_trivial = expected_storage_base<Trivial_t, Err>;
using Base_trivial = expected_storage_base<Trivial_t, Trivial_e>;

using Base_void_not_trivial = expected_storage_base<void, Err>;
using Base_void_trivial = expected_storage_base<void, Trivial_e>;

} // namespace

TEST(expected_storage_base, type_traits) {
  ASSERT_FALSE(std::is_trivially_destructible_v<Val>);
  ASSERT_FALSE(std::is_trivially_destructible_v<Err>);
  ASSERT_TRUE(std::is_trivially_destructible_v<Trivial_t>);
  ASSERT_TRUE(std::is_trivially_destructible_v<Trivial_e>);

  ASSERT_FALSE(std::is_trivially_destructible_v<Base_not_trivial>);
  ASSERT_FALSE(std::is_trivially_destructible_v<Base_t_not_trivial>);
  ASSERT_FALSE(std::is_trivially_destructible_v<Base_e_not_trivial>);
  ASSERT_TRUE(std::is_trivially_destructible_v<Base_trivial>);

  ASSERT_FALSE(std::is_trivially_destructible_v<Base_void_not_trivial>);
  ASSERT_TRUE(std::is_trivially_destructible_v<Base_void_trivial>);
}

TEST(expected_storage_base, default_constructor) {
  Val::reset();
  Err::reset();
  {
    Base_not_trivial b;
    ASSERT_EQ(b.val_.x, 20100);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Val::s, State::default_constructed);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  {
    Base_t_not_trivial b;
    ASSERT_EQ(b.val_.x, 20100);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Val::s, State::default_constructed);
  }
  ASSERT_EQ(Val::s, State::destructed);
  Val::reset();
  {
    Base_e_not_trivial b;
    ASSERT_EQ(b.val_.x, 20100);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::none);
  {
    Base_trivial b;
    ASSERT_EQ(b.val_.x, 20100);
    ASSERT_TRUE(b.has_val_);
  }
  // T is void
  {
    Base_void_not_trivial b;
    (void)b.dummy_;
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::none);
  {
    Base_void_trivial b;
    (void)b.dummy_;
    ASSERT_TRUE(b.has_val_);
  }
}

TEST(expected_storage_base, uninit_t_constructor) {
  Val::reset();
  Err::reset();
  {
    Base_not_trivial b(uninit);
    ASSERT_EQ(b.uninit_, '\0');
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Base_t_not_trivial b(uninit);
    ASSERT_EQ(b.uninit_, '\0');
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(Val::s, State::none);
  }
  ASSERT_EQ(Val::s, State::none);
  {
    Base_e_not_trivial b(uninit);
    ASSERT_EQ(b.uninit_, '\0');
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Base_trivial b(uninit);
    ASSERT_EQ(b.uninit_, '\0');
    ASSERT_FALSE(b.has_val_);
  }
  // T is void
  {
    Base_void_not_trivial b(uninit);
    ASSERT_EQ(b.uninit_, '\0');
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Base_void_trivial b(uninit);
    ASSERT_EQ(b.uninit_, '\0');
    ASSERT_FALSE(b.has_val_);
  }
}

TEST(expected_storage_base, in_place_t_constructor) {
  Val::reset();
  Err::reset();
  {
    Arg arg(1);
    Base_not_trivial b(std::in_place, std::move(arg), 1);
    ASSERT_EQ(b.val_.x, 1);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::constructed);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  {
    Arg arg(2);
    Base_t_not_trivial b(std::in_place, std::move(arg), 2);
    ASSERT_EQ(b.val_.x, 2);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::constructed);
  }
  ASSERT_EQ(Val::s, State::destructed);
  Val::reset();
  {
    Arg arg(3);
    Base_e_not_trivial b(std::in_place, std::move(arg), 3);
    ASSERT_EQ(b.val_.x, 3);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::none);
  {
    Arg arg(4);
    Base_trivial b(std::in_place, std::move(arg), 4);
    ASSERT_EQ(b.val_.x, 4);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
  // T is void
  {
    Base_void_not_trivial b(std::in_place);
    (void)b.dummy_;
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::none);
  {
    Base_void_trivial b(std::in_place);
    (void)b.dummy_;
    ASSERT_TRUE(b.has_val_);
  }
}

TEST(expected_storage_base, unexpect_t_constructor) {
  Val::reset();
  Err::reset();
  {
    Arg arg(1);
    Base_not_trivial b(exp::unexpect, std::move(arg), 1);
    ASSERT_EQ(b.unexpect_.value().x, 1);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::constructed);
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(2);
    Base_t_not_trivial b(exp::unexpect, std::move(arg), 2);
    ASSERT_EQ(b.unexpect_.value().x, 2);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::none);
  }
  ASSERT_EQ(Val::s, State::none);
  {
    Arg arg(3);
    Base_e_not_trivial b(exp::unexpect, std::move(arg), 3);
    ASSERT_EQ(b.unexpect_.value().x, 3);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::constructed);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(4);
    Base_trivial b(exp::unexpect, std::move(arg), 4);
    ASSERT_EQ(b.unexpect_.value().x, 4);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
  // T is void
  {
    Arg arg(5);
    Base_void_not_trivial b(exp::unexpect, std::move(arg), 5);
    ASSERT_EQ(b.unexpect_.value().x, 5);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::constructed);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(6);
    Base_void_trivial b(exp::unexpect, std::move(arg), 6);
    ASSERT_EQ(b.unexpect_.value().x, 6);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
}

TEST(expected_storage_base, in_place_t_initializer_list_constructor) {
  Val::reset();
  Err::reset();
  {
    Arg arg(1);
    Base_not_trivial b(std::in_place, {1}, std::move(arg), 1);
    ASSERT_EQ(b.val_.x, 1 + 1);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::constructed);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  {
    Arg arg(2);
    Base_t_not_trivial b(std::in_place, {2}, std::move(arg), 2);
    ASSERT_EQ(b.val_.x, 2 + 2);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::constructed);
  }
  ASSERT_EQ(Val::s, State::destructed);
  Val::reset();
  {
    Arg arg(3);
    Base_e_not_trivial b(std::in_place, {3}, std::move(arg), 3);
    ASSERT_EQ(b.val_.x, 3 + 3);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::none);
  {
    Arg arg(4);
    Base_trivial b(std::in_place, {4}, std::move(arg), 4);
    ASSERT_EQ(b.val_.x, 4 + 4);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
}

TEST(expected_storage_base, unexpect_t_initializer_list_constructor) {
  Val::reset();
  Err::reset();
  {
    Arg arg(1);
    Base_not_trivial b(exp::unexpect, {1}, std::move(arg), 1);
    ASSERT_EQ(b.unexpect_.value().x, 1 + 1);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::constructed);
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(2);
    Base_t_not_trivial b(exp::unexpect, {2}, std::move(arg), 2);
    ASSERT_EQ(b.unexpect_.value().x, 2 + 2);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::none);
  }
  ASSERT_EQ(Val::s, State::none);
  {
    Arg arg(3);
    Base_e_not_trivial b(exp::unexpect, {3}, std::move(arg), 3);
    ASSERT_EQ(b.unexpect_.value().x, 3 + 3);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::constructed);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(4);
    Base_trivial b(exp::unexpect, {4}, std::move(arg), 4);
    ASSERT_EQ(b.unexpect_.value().x, 4 + 4);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
  // T is void
  {
    Arg arg(5);
    Base_void_not_trivial b(exp::unexpect, {5}, std::move(arg), 5);
    ASSERT_EQ(b.unexpect_.value().x, 5 + 5);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::constructed);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(6);
    Base_void_trivial b(exp::unexpect, {6}, std::move(arg), 6);
    ASSERT_EQ(b.unexpect_.value().x, 6 + 6);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
}
