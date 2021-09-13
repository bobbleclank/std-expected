#include "exp/expected.h"

#include <gtest/gtest.h>

using namespace exp;

namespace {

enum class State { none, default_constructed, destructed };

template <class Tag> struct Obj {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  Obj() { s = State::default_constructed; }

  ~Obj() { s = State::destructed; }

  int x = 0;
};

struct Val_tag {};
using Val = Obj<Val_tag>;

struct Err_tag {};
using Err = Obj<Err_tag>;

} // namespace

TEST(expected, has_value) {
  {
    expected<Val, Err> e;
    ASSERT_TRUE(static_cast<bool>(e));
    ASSERT_TRUE(e.has_value());
  }
}

TEST(expected, default_constructor) {
  Val::reset();
  Err::reset();
  {
    expected<Val, Err> e;
    ASSERT_EQ(Val::s, State::default_constructed);
    ASSERT_EQ(Err::s, State::none);
    ASSERT_EQ(e->x, 0);
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
}
