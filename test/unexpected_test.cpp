#include "exp/expected.h"

#include <cstddef>
#include <initializer_list>
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

  Err(Err&& other) noexcept {
    e = other.e;
    other.e = -1;
  }

  Err& operator=(Err&& other) noexcept {
    e = other.e;
    other.e = -2;
    return *this;
  }

  int e = 0;
};

bool operator==(Err lhs, Err rhs) { return lhs.e == rhs.e; }
bool operator!=(Err lhs, Err rhs) { return !(lhs == rhs); }

struct Err2 {
  Err2() = default;
  explicit Err2(int e_) : e(e_) {}
  explicit Err2(const Err& err_) : err(err_) {}
  explicit Err2(Err&& err_) : err(std::move(err_)) {}
  Err2(int e_, Err&& err_) : e(e_), err(std::move(err_)) {}

  Err2(std::initializer_list<int> il, int e_, Err&& err_)
      : il_size(il.size()), e(e_), err(std::move(err_)) {}

  std::size_t il_size = 0;
  int e = 0;
  Err err;
};

bool operator==(Err2 lhs, Err rhs) { return lhs.e == rhs.e; }
bool operator!=(Err2 lhs, Err rhs) { return !(lhs == rhs); }

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
    // Since the r-value reference is const, Err's copy constructor is called,
    // and not Err's move constructor (which takes a non-const r-value
    // reference).
  }
  {
    const unexpected<Err> e(4);
    Err err;
    err = std::move(e).value();
    ASSERT_EQ(err.e, 4);
    ASSERT_EQ(e.value().e, 4);
    // Since the r-value reference is const, Err's copy assignment is called,
    // and not Err's move assignment (which takes a non-const r-value
    // reference).
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
  // (std::in_place_t, Args&&...)
  {
    Err val(11);
    unexpected<Err2> e(std::in_place, 10, std::move(val));
    ASSERT_EQ(e.value().e, 10);
    ASSERT_EQ(e.value().err.e, 11);
    ASSERT_EQ(val.e, -1);
  }
  // (std::in_place_t, std::initializer_list<U>, Args&&...)
  {
    Err val(20);
    unexpected<Err2> e(std::in_place, {18}, 19, std::move(val));
    ASSERT_EQ(e.value().il_size, 1);
    ASSERT_EQ(e.value().e, 19);
    ASSERT_EQ(e.value().err.e, 20);
    ASSERT_EQ(val.e, -1);
  }
}

TEST(unexpected, equality_operators) {
  unexpected<Err> e_one(1);

  unexpected<Err> e1(1);
  unexpected<Err> e2(2);

  unexpected<Err2> e_two(2);

  // Operands have same type.

  ASSERT_TRUE(e_one == e1);
  ASSERT_FALSE(e_one == e2);
  ASSERT_FALSE(e_one != e1);
  ASSERT_TRUE(e_one != e2);

  // Operands have different types.

  ASSERT_FALSE(e_two == e1);
  ASSERT_TRUE(e_two == e2);
  ASSERT_TRUE(e_two != e1);
  ASSERT_FALSE(e_two != e2);
}
