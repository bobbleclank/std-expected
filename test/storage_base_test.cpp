#include "exp/expected.h"

#include "arg.h"

#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

using namespace exp::internal;

namespace {

enum class State { none, default_constructed, constructed, destructed };

template <class Tag> struct Not_trivial {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  Not_trivial() { s = State::default_constructed; }

  Not_trivial(Arg&& arg_, int) {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  ~Not_trivial() { s = State::destructed; }

  int x = 0;
};

template <class Tag> struct Trivial {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  Trivial() { s = State::default_constructed; }

  Trivial(Arg&& arg_, int) {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  ~Trivial() = default;

  int x = 0;
};

struct Not_trivial_t_tag {};
using Not_trivial_t = Not_trivial<Not_trivial_t_tag>;

struct Not_trivial_e_tag {};
using Not_trivial_e = Not_trivial<Not_trivial_e_tag>;

struct Trivial_t_tag {};
using Trivial_t = Trivial<Trivial_t_tag>;

struct Trivial_e_tag {};
using Trivial_e = Trivial<Trivial_e_tag>;

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
  Trivial_t::reset();
  Trivial_e::reset();
  {
    Base_not_trivial b;
    ASSERT_EQ(b.val_.x, 0);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Not_trivial_t::s, State::default_constructed);
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_t::s, State::destructed);
  ASSERT_EQ(Not_trivial_e::s, State::none);
  Not_trivial_t::reset();
  {
    Base_t_not_trivial b;
    ASSERT_EQ(b.val_.x, 0);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Not_trivial_t::s, State::default_constructed);
    ASSERT_EQ(Trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_t::s, State::destructed);
  ASSERT_EQ(Trivial_e::s, State::none);
  Not_trivial_t::reset();
  {
    Base_e_not_trivial b;
    ASSERT_EQ(b.val_.x, 0);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Trivial_t::s, State::default_constructed);
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Trivial_t::s, State::default_constructed);
  ASSERT_EQ(Not_trivial_e::s, State::none);
  Trivial_t::reset();
  {
    Base_trivial b;
    ASSERT_EQ(b.val_.x, 0);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Trivial_t::s, State::default_constructed);
    ASSERT_EQ(Trivial_e::s, State::none);
  }
  ASSERT_EQ(Trivial_t::s, State::default_constructed);
  ASSERT_EQ(Trivial_e::s, State::none);
  Trivial_t::reset();
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
    ASSERT_EQ(Trivial_e::s, State::none);
  }
  ASSERT_EQ(Trivial_e::s, State::none);
}

TEST(expected_storage_base, uninit_t_constructor) {
  Not_trivial_t::reset();
  Not_trivial_e::reset();
  Trivial_t::reset();
  Trivial_e::reset();
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
    ASSERT_EQ(Trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_t::s, State::none);
  ASSERT_EQ(Trivial_e::s, State::none);
  {
    Base_e_not_trivial b(uninit);
    ASSERT_EQ(b.uninit_, '\0');
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(Trivial_t::s, State::none);
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Trivial_t::s, State::none);
  ASSERT_EQ(Not_trivial_e::s, State::destructed);
  Not_trivial_e::reset();
  {
    Base_trivial b(uninit);
    ASSERT_EQ(b.uninit_, '\0');
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(Trivial_t::s, State::none);
    ASSERT_EQ(Trivial_e::s, State::none);
  }
  ASSERT_EQ(Trivial_t::s, State::none);
  ASSERT_EQ(Trivial_e::s, State::none);
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
    ASSERT_EQ(Trivial_e::s, State::none);
  }
  ASSERT_EQ(Trivial_e::s, State::none);
}

TEST(expected_storage_base, in_place_t_constructor) {
  Not_trivial_t::reset();
  Not_trivial_e::reset();
  Trivial_t::reset();
  Trivial_e::reset();
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
    ASSERT_EQ(Trivial_e::s, State::none);
  }
  ASSERT_EQ(Not_trivial_t::s, State::destructed);
  ASSERT_EQ(Trivial_e::s, State::none);
  Not_trivial_t::reset();
  {
    Arg arg(3);
    Base_e_not_trivial b(std::in_place, std::move(arg), 3);
    ASSERT_EQ(b.val_.x, 3);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Trivial_t::s, State::constructed);
    ASSERT_EQ(Not_trivial_e::s, State::none);
  }
  ASSERT_EQ(Trivial_t::s, State::constructed);
  ASSERT_EQ(Not_trivial_e::s, State::none);
  Trivial_t::reset();
  {
    Arg arg(4);
    Base_trivial b(std::in_place, std::move(arg), 4);
    ASSERT_EQ(b.val_.x, 4);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Trivial_t::s, State::constructed);
    ASSERT_EQ(Trivial_e::s, State::none);
  }
  ASSERT_EQ(Trivial_t::s, State::constructed);
  ASSERT_EQ(Trivial_e::s, State::none);
  Trivial_t::reset();
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
    ASSERT_EQ(Trivial_e::s, State::none);
  }
  ASSERT_EQ(Trivial_e::s, State::none);
}

TEST(expected_storage_base, unexpect_t_constructor) {
  Not_trivial_t::reset();
  Not_trivial_e::reset();
  Trivial_t::reset();
  Trivial_e::reset();
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
    ASSERT_EQ(Trivial_e::s, State::constructed);
  }
  ASSERT_EQ(Not_trivial_t::s, State::none);
  ASSERT_EQ(Trivial_e::s, State::constructed);
  Trivial_e::reset();
  {
    Arg arg(3);
    Base_e_not_trivial b(exp::unexpect, std::move(arg), 3);
    ASSERT_EQ(b.unexpect_.value().x, 3);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Trivial_t::s, State::none);
    ASSERT_EQ(Not_trivial_e::s, State::constructed);
  }
  ASSERT_EQ(Trivial_t::s, State::none);
  ASSERT_EQ(Not_trivial_e::s, State::destructed);
  Not_trivial_e::reset();
  {
    Arg arg(4);
    Base_trivial b(exp::unexpect, std::move(arg), 4);
    ASSERT_EQ(b.unexpect_.value().x, 4);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Trivial_t::s, State::none);
    ASSERT_EQ(Trivial_e::s, State::constructed);
  }
  ASSERT_EQ(Trivial_t::s, State::none);
  ASSERT_EQ(Trivial_e::s, State::constructed);
  Trivial_e::reset();
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
    ASSERT_EQ(Trivial_e::s, State::constructed);
  }
  ASSERT_EQ(Trivial_e::s, State::constructed);
  Trivial_e::reset();
}
