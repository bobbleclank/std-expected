#include "exp/expected.h"

#include <utility>

#include <gtest/gtest.h>

using namespace exp;

namespace {

struct Err {
  Err() = default;
  explicit Err(int e_) : e(e_) {}
  ~Err() = default;

  Err(const Err&) = default;
  Err& operator=(const Err&) = default;

  Err(Err&& other) : e(other.e) { other.e = -1; }

  Err& operator=(Err&& other) {
    e = other.e;
    other.e = -2;
    return *this;
  }

  int e = 0;
};

struct Err2 {
  Err2() = default;
  explicit Err2(const Err& err_) : err(err_) {}
  explicit Err2(Err&& err_) : err(std::move(err_)) {}

  Err err;
};

} // namespace

TEST(unexpected, value) {
  // const& overload
  {
    const unexpected<Err> e(1);
    const Err& err = e.value();
    ASSERT_EQ(err.e, 1);
    ASSERT_EQ(e.value().e, 1);
  }
  // non-const& overload
  {
    unexpected<Err> e(2);
    Err& err = e.value();
    ASSERT_EQ(err.e, 2);
    ASSERT_EQ(e.value().e, 2);
    err.e = 20;
    ASSERT_EQ(err.e, 20);
    ASSERT_EQ(e.value().e, 20);
  }
  // const&& overload
  {
    const unexpected<Err> e(3);
    Err err = std::move(e).value();
    ASSERT_EQ(err.e, 3);
    ASSERT_EQ(e.value().e, 3);
    // Is this correct? I need to better understand const&&.
  }
  {
    const unexpected<Err> e(4);
    Err err;
    err = std::move(e).value();
    ASSERT_EQ(err.e, 4);
    ASSERT_EQ(e.value().e, 4);
    // Is this correct? I need to better understand const&&.
  }
  // non-const&& overload
  {
    unexpected<Err> e(5);
    Err err = std::move(e).value();
    ASSERT_EQ(err.e, 5);
    ASSERT_EQ(e.value().e, -1);
  }
  {
    unexpected<Err> e(6);
    Err err;
    err = std::move(e).value();
    ASSERT_EQ(err.e, 6);
    ASSERT_EQ(e.value().e, -2);
  }
}

TEST(unexpected, constructors) {
  // Deduction guide
  {
    Err val(1);
    unexpected e(val);
    ASSERT_EQ(e.value().e, 1);
    ASSERT_EQ(val.e, 1);
  }
  // (Err&&) with Err = E
  {
    Err val(2);
    unexpected<Err> e(val);
    ASSERT_EQ(e.value().e, 2);
    ASSERT_EQ(val.e, 2);
  }
  {
    Err val(3);
    unexpected<Err> e(std::move(val));
    ASSERT_EQ(e.value().e, 3);
    ASSERT_EQ(val.e, -1);
  }
  // (Err&&) with Err != E
  {
    int val = 4;
    unexpected<Err> e(val);
    ASSERT_EQ(e.value().e, 4);
    ASSERT_EQ(val, 4);
  }
  {
    Err val(5);
    unexpected<Err2> e(val);
    ASSERT_EQ(e.value().err.e, 5);
    ASSERT_EQ(val.e, 5);
  }
  {
    Err val(6);
    unexpected<Err2> e(std::move(val));
    ASSERT_EQ(e.value().err.e, 6);
    ASSERT_EQ(val.e, -1);
  }
}
