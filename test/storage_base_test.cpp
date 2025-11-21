#include "bc/expected.h"

#include "arg.h"
#include "obj.h"
#include "obj_trivial.h"
#include "state.h"
#include "union_access.h"

#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

using namespace bc;

namespace {

using Base = detail::expected_storage_base<Val, Err>;
using B_e_trivial = detail::expected_storage_base<Val, Err_trivial>;
using B_t_trivial = detail::expected_storage_base<Val_trivial, Err>;
using B_trivial = detail::expected_storage_base<Val_trivial, Err_trivial>;

using Base_void = detail::expected_storage_base<void, Err>;
using B_void_trivial = detail::expected_storage_base<void, Err_trivial>;

} // namespace

// NOLINTBEGIN(*-avoid-magic-numbers): Test values
// NOLINTBEGIN(clang-analyzer-cplusplus.Move)

TEST(expected_storage_base, type_traits) {
  ASSERT_FALSE(std::is_trivially_destructible_v<Val>);
  ASSERT_FALSE(std::is_trivially_destructible_v<Err>);
  ASSERT_TRUE(std::is_trivially_destructible_v<Val_trivial>);
  ASSERT_TRUE(std::is_trivially_destructible_v<Err_trivial>);

  ASSERT_FALSE(std::is_trivially_destructible_v<Base>);
  ASSERT_FALSE(std::is_trivially_destructible_v<B_e_trivial>);
  ASSERT_FALSE(std::is_trivially_destructible_v<B_t_trivial>);
  ASSERT_TRUE(std::is_trivially_destructible_v<B_trivial>);

  ASSERT_FALSE(std::is_trivially_destructible_v<Base_void>);
  ASSERT_TRUE(std::is_trivially_destructible_v<B_void_trivial>);
}

TEST(expected_storage_base, default_constructor) {
  Val::reset();
  Err::reset();
  {
    Base b;
    ASSERT_EQ(val(b).x, 20100);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Val::s, State::default_constructed);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  {
    B_e_trivial b;
    ASSERT_EQ(val(b).x, 20100);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Val::s, State::default_constructed);
  }
  ASSERT_EQ(Val::s, State::destructed);
  Val::reset();
  {
    B_t_trivial b;
    ASSERT_EQ(val(b).x, 20100);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::none);
  {
    B_trivial b;
    ASSERT_EQ(val(b).x, 20100);
    ASSERT_TRUE(b.has_val_);
  }
  // T is void
  {
    Base_void b;
    (void)dummy(b);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::none);
  {
    B_void_trivial b;
    (void)dummy(b);
    ASSERT_TRUE(b.has_val_);
  }
}

TEST(expected_storage_base, uninit_t_constructor) {
  Val::reset();
  Err::reset();
  {
    Base b(detail::uninit);
    ASSERT_EQ(uninit(b), '\0');
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    B_e_trivial b(detail::uninit);
    ASSERT_EQ(uninit(b), '\0');
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(Val::s, State::none);
  }
  ASSERT_EQ(Val::s, State::none);
  {
    B_t_trivial b(detail::uninit);
    ASSERT_EQ(uninit(b), '\0');
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    B_trivial b(detail::uninit);
    ASSERT_EQ(uninit(b), '\0');
    ASSERT_FALSE(b.has_val_);
  }
  // T is void
  {
    Base_void b(detail::uninit);
    ASSERT_EQ(uninit(b), '\0');
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    B_void_trivial b(detail::uninit);
    ASSERT_EQ(uninit(b), '\0');
    ASSERT_FALSE(b.has_val_);
  }
}

TEST(expected_storage_base, in_place_t_constructor) {
  Val::reset();
  Err::reset();
  {
    Arg arg(1);
    Base b(std::in_place, std::move(arg), 1);
    ASSERT_EQ(val(b).x, 1 + 1);
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
    B_e_trivial b(std::in_place, std::move(arg), 2);
    ASSERT_EQ(val(b).x, 2 + 2);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::constructed);
  }
  ASSERT_EQ(Val::s, State::destructed);
  Val::reset();
  {
    Arg arg(3);
    B_t_trivial b(std::in_place, std::move(arg), 3);
    ASSERT_EQ(val(b).x, 3 + 3);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::none);
  {
    Arg arg(4);
    B_trivial b(std::in_place, std::move(arg), 4);
    ASSERT_EQ(val(b).x, 4 + 4);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
  // T is void
  {
    Base_void b(std::in_place);
    (void)dummy(b);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::none);
  {
    B_void_trivial b(std::in_place);
    (void)dummy(b);
    ASSERT_TRUE(b.has_val_);
  }
}

TEST(expected_storage_base, unexpect_t_constructor) {
  Val::reset();
  Err::reset();
  {
    Arg arg(1);
    Base b(unexpect, std::move(arg), 1);
    ASSERT_EQ(err(b).x, 1 + 1);
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
    B_e_trivial b(unexpect, std::move(arg), 2);
    ASSERT_EQ(err(b).x, 2 + 2);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::none);
  }
  ASSERT_EQ(Val::s, State::none);
  {
    Arg arg(3);
    B_t_trivial b(unexpect, std::move(arg), 3);
    ASSERT_EQ(err(b).x, 3 + 3);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::constructed);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(4);
    B_trivial b(unexpect, std::move(arg), 4);
    ASSERT_EQ(err(b).x, 4 + 4);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
  // T is void
  {
    Arg arg(5);
    Base_void b(unexpect, std::move(arg), 5);
    ASSERT_EQ(err(b).x, 5 + 5);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::constructed);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(6);
    B_void_trivial b(unexpect, std::move(arg), 6);
    ASSERT_EQ(err(b).x, 6 + 6);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
}

TEST(expected_storage_base, in_place_t_initializer_list_constructor) {
  Val::reset();
  Err::reset();
  {
    Arg arg(1);
    Base b(std::in_place, {1}, std::move(arg), 1);
    ASSERT_EQ(val(b).x, 1 + 1 + 1);
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
    B_e_trivial b(std::in_place, {2}, std::move(arg), 2);
    ASSERT_EQ(val(b).x, 2 + 2 + 2);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::constructed);
  }
  ASSERT_EQ(Val::s, State::destructed);
  Val::reset();
  {
    Arg arg(3);
    B_t_trivial b(std::in_place, {3}, std::move(arg), 3);
    ASSERT_EQ(val(b).x, 3 + 3 + 3);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::none);
  {
    Arg arg(4);
    B_trivial b(std::in_place, {4}, std::move(arg), 4);
    ASSERT_EQ(val(b).x, 4 + 4 + 4);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
}

TEST(expected_storage_base, unexpect_t_initializer_list_constructor) {
  Val::reset();
  Err::reset();
  {
    Arg arg(1);
    Base b(unexpect, {1}, std::move(arg), 1);
    ASSERT_EQ(err(b).x, 1 + 1 + 1);
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
    B_e_trivial b(unexpect, {2}, std::move(arg), 2);
    ASSERT_EQ(err(b).x, 2 + 2 + 2);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::none);
  }
  ASSERT_EQ(Val::s, State::none);
  {
    Arg arg(3);
    B_t_trivial b(unexpect, {3}, std::move(arg), 3);
    ASSERT_EQ(err(b).x, 3 + 3 + 3);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::constructed);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(4);
    B_trivial b(unexpect, {4}, std::move(arg), 4);
    ASSERT_EQ(err(b).x, 4 + 4 + 4);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
  // T is void
  {
    Arg arg(5);
    Base_void b(unexpect, {5}, std::move(arg), 5);
    ASSERT_EQ(err(b).x, 5 + 5 + 5);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::constructed);
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  {
    Arg arg(6);
    B_void_trivial b(unexpect, {6}, std::move(arg), 6);
    ASSERT_EQ(err(b).x, 6 + 6 + 6);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
  }
}

// NOLINTEND(clang-analyzer-cplusplus.Move)
// NOLINTEND(*-avoid-magic-numbers): Test values
