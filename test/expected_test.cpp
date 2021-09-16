#include "exp/expected.h"

#include <initializer_list>
#include <utility>

#include <gtest/gtest.h>

using namespace exp;

namespace {

enum class State {
  none,
  default_constructed,
  args_constructed,
  list_constructed,
  destructed
};

struct Arg {
  Arg() = default;
  explicit Arg(int x_) : x(x_) {}

  int x = 0;
};

template <class Tag> struct Obj {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  Obj() { s = State::default_constructed; }

  Obj(const Arg& arg_, int) {
    s = State::args_constructed;
    Arg arg = arg_;
    x = arg.x;
  }

  Obj(std::initializer_list<int>, const Arg& arg_, int) {
    s = State::list_constructed;
    Arg arg = arg_;
    x = arg.x;
  }

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
  {
    expected<Val, Err> e(std::in_place, Arg(1), 1);
    ASSERT_TRUE(static_cast<bool>(e));
    ASSERT_TRUE(e.has_value());
  }
  {
    expected<Val, Err> e(std::in_place, {2}, Arg(2), 2);
    ASSERT_TRUE(static_cast<bool>(e));
    ASSERT_TRUE(e.has_value());
  }
  {
    expected<Val, Err> e(unexpect, Arg(3), 3);
    ASSERT_FALSE(static_cast<bool>(e));
    ASSERT_FALSE(e.has_value());
  }
  {
    expected<Val, Err> e(unexpect, {4}, Arg(4), 4);
    ASSERT_FALSE(static_cast<bool>(e));
    ASSERT_FALSE(e.has_value());
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
