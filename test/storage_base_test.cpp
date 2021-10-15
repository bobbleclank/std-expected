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

struct Not_trivial_t_tag {};
using Not_trivial_t = Obj<Not_trivial_t_tag>;

struct Not_trivial_e_tag {};
using Not_trivial_e = Obj<Not_trivial_e_tag>;

struct Trivial_t_tag {};
using Trivial_t = Obj_trivial<Trivial_t_tag>;

struct Trivial_e_tag {};
using Trivial_e = Obj_trivial<Trivial_e_tag>;

using Base_not_trivial = expected_storage_base<Not_trivial_t, Not_trivial_e>;
using Base_t_not_trivial = expected_storage_base<Not_trivial_t, Trivial_e>;
using Base_e_not_trivial = expected_storage_base<Trivial_t, Not_trivial_e>;
using Base_trivial = expected_storage_base<Trivial_t, Trivial_e>;

using Base_void_not_trivial = expected_storage_base<void, Not_trivial_e>;
using Base_void_trivial = expected_storage_base<void, Trivial_e>;

} // namespace

TEST(expected_storage_base, type_traits) {
  ASSERT_FALSE(std::is_trivially_destructible_v<Not_trivial_t>);
  ASSERT_FALSE(std::is_trivially_destructible_v<Not_trivial_e>);
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
  Not_trivial_t::reset();
  Not_trivial_e::reset();
  {
    Base_not_trivial b;
    ASSERT_EQ(b.val_.x, 20100);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Not_trivial_t::s, State::default_constructed);
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_t::s, State::destructed);
  ASSERT_EQ(Not_trivial_e::s, State::none);
  Not_trivial_t::reset();
  {
    Base_t_not_trivial b;
    ASSERT_EQ(b.val_.x, 20100);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Not_trivial_t::s, State::default_constructed);
  }
  ASSERT_EQ(Not_trivial_t::s, State::destructed);
  Not_trivial_t::reset();
  {
    Base_e_not_trivial b;
    ASSERT_EQ(b.val_.x, 20100);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_e::s, State::none);
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
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_e::s, State::none);
  {
    Base_void_trivial b;
    (void)b.dummy_;
    ASSERT_TRUE(b.has_val_);
  }
}

TEST(expected_storage_base, uninit_t_constructor) {
  Not_trivial_t::reset();
  Not_trivial_e::reset();
  {
    Base_not_trivial b(uninit);
    ASSERT_EQ(b.uninit_, '\0');
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(Not_trivial_t::s, State::none);
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_t::s, State::none);
  ASSERT_EQ(Not_trivial_e::s, State::destructed);
  Not_trivial_e::reset();
  {
    Base_t_not_trivial b(uninit);
    ASSERT_EQ(b.uninit_, '\0');
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(Not_trivial_t::s, State::none);
  }
  ASSERT_EQ(Not_trivial_t::s, State::none);
  {
    Base_e_not_trivial b(uninit);
    ASSERT_EQ(b.uninit_, '\0');
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_e::s, State::destructed);
  Not_trivial_e::reset();
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
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_e::s, State::destructed);
  Not_trivial_e::reset();
  {
    Base_void_trivial b(uninit);
    ASSERT_EQ(b.uninit_, '\0');
    ASSERT_FALSE(b.has_val_);
  }
}

TEST(expected_storage_base, in_place_t_constructor) {
  Not_trivial_t::reset();
  Not_trivial_e::reset();
  {
    Arg arg(1);
    Base_not_trivial b(std::in_place, std::move(arg), 1);
    ASSERT_EQ(b.val_.x, 1);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Not_trivial_t::s, State::constructed);
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_t::s, State::destructed);
  ASSERT_EQ(Not_trivial_e::s, State::none);
  Not_trivial_t::reset();
  {
    Arg arg(2);
    Base_t_not_trivial b(std::in_place, std::move(arg), 2);
    ASSERT_EQ(b.val_.x, 2);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Not_trivial_t::s, State::constructed);
  }
  ASSERT_EQ(Not_trivial_t::s, State::destructed);
  Not_trivial_t::reset();
  {
    Arg arg(3);
    Base_e_not_trivial b(std::in_place, std::move(arg), 3);
    ASSERT_EQ(b.val_.x, 3);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_e::s, State::none);
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
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_e::s, State::none);
  {
    Base_void_trivial b(std::in_place);
    (void)b.dummy_;
    ASSERT_TRUE(b.has_val_);
  }
}

TEST(expected_storage_base, unexpect_t_constructor) {
  Not_trivial_t::reset();
  Not_trivial_e::reset();
  {
    Arg arg(1);
    Base_not_trivial b(exp::unexpect, std::move(arg), 1);
    ASSERT_EQ(b.unexpect_.value().x, 1);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Not_trivial_t::s, State::none);
    ASSERT_EQ(Not_trivial_e::s, State::constructed);
  }
  ASSERT_EQ(Not_trivial_t::s, State::none);
  ASSERT_EQ(Not_trivial_e::s, State::destructed);
  Not_trivial_e::reset();
  {
    Arg arg(2);
    Base_t_not_trivial b(exp::unexpect, std::move(arg), 2);
    ASSERT_EQ(b.unexpect_.value().x, 2);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Not_trivial_t::s, State::none);
  }
  ASSERT_EQ(Not_trivial_t::s, State::none);
  {
    Arg arg(3);
    Base_e_not_trivial b(exp::unexpect, std::move(arg), 3);
    ASSERT_EQ(b.unexpect_.value().x, 3);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Not_trivial_e::s, State::constructed);
  }
  ASSERT_EQ(Not_trivial_e::s, State::destructed);
  Not_trivial_e::reset();
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
    ASSERT_EQ(Not_trivial_e::s, State::constructed);
  }
  ASSERT_EQ(Not_trivial_e::s, State::destructed);
  Not_trivial_e::reset();
  {
    Arg arg(6);
    Base_void_trivial b(exp::unexpect, std::move(arg), 6);
    ASSERT_EQ(b.unexpect_.value().x, 6);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
}

TEST(expected_storage_base, in_place_t_initializer_list_constructor) {
  Not_trivial_t::reset();
  Not_trivial_e::reset();
  {
    Arg arg(1);
    Base_not_trivial b(std::in_place, {1}, std::move(arg), 1);
    ASSERT_EQ(b.val_.x, 1 + 1);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Not_trivial_t::s, State::constructed);
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_t::s, State::destructed);
  ASSERT_EQ(Not_trivial_e::s, State::none);
  Not_trivial_t::reset();
  {
    Arg arg(2);
    Base_t_not_trivial b(std::in_place, {2}, std::move(arg), 2);
    ASSERT_EQ(b.val_.x, 2 + 2);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Not_trivial_t::s, State::constructed);
  }
  ASSERT_EQ(Not_trivial_t::s, State::destructed);
  Not_trivial_t::reset();
  {
    Arg arg(3);
    Base_e_not_trivial b(std::in_place, {3}, std::move(arg), 3);
    ASSERT_EQ(b.val_.x, 3 + 3);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_e::s, State::none);
  {
    Arg arg(4);
    Base_trivial b(std::in_place, {4}, std::move(arg), 4);
    ASSERT_EQ(b.val_.x, 4 + 4);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
}

TEST(expected_storage_base, unexpect_t_initializer_list_constructor) {
  Not_trivial_t::reset();
  Not_trivial_e::reset();
  {
    Arg arg(1);
    Base_not_trivial b(exp::unexpect, {1}, std::move(arg), 1);
    ASSERT_EQ(b.unexpect_.value().x, 1 + 1);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Not_trivial_t::s, State::none);
    ASSERT_EQ(Not_trivial_e::s, State::constructed);
  }
  ASSERT_EQ(Not_trivial_t::s, State::none);
  ASSERT_EQ(Not_trivial_e::s, State::destructed);
  Not_trivial_e::reset();
  {
    Arg arg(2);
    Base_t_not_trivial b(exp::unexpect, {2}, std::move(arg), 2);
    ASSERT_EQ(b.unexpect_.value().x, 2 + 2);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Not_trivial_t::s, State::none);
  }
  ASSERT_EQ(Not_trivial_t::s, State::none);
  {
    Arg arg(3);
    Base_e_not_trivial b(exp::unexpect, {3}, std::move(arg), 3);
    ASSERT_EQ(b.unexpect_.value().x, 3 + 3);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Not_trivial_e::s, State::constructed);
  }
  ASSERT_EQ(Not_trivial_e::s, State::destructed);
  Not_trivial_e::reset();
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
    ASSERT_EQ(Not_trivial_e::s, State::constructed);
  }
  ASSERT_EQ(Not_trivial_e::s, State::destructed);
  Not_trivial_e::reset();
  {
    Arg arg(6);
    Base_void_trivial b(exp::unexpect, {6}, std::move(arg), 6);
    ASSERT_EQ(b.unexpect_.value().x, 6 + 6);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
}
