#include "exp/expected.h"

#include <initializer_list>
#include <utility>
#include <vector>

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

bool operator==(Err lhs, Err rhs) { return lhs.e == rhs.e; }
bool operator!=(Err lhs, Err rhs) { return !(lhs == rhs); }

struct Err2 {
  Err2() = default;
  explicit Err2(const Err& err_) : err(err_) {}
  explicit Err2(Err&& err_) : err(std::move(err_)) {}

  Err err;
};

bool operator==(Err lhs, Err2 rhs) { return lhs.e == rhs.err.e; }
bool operator!=(Err lhs, Err2 rhs) { return !(lhs == rhs); }

struct Err3 {
  Err3() = default;
  explicit Err3(int e_) : e(e_) {}
  Err3(int e_, const Err& err_) : e(e_), err(err_) {}
  Err3(int e_, Err&& err_) : e(e_), err(std::move(err_)) {}

  explicit Err3(std::initializer_list<int> il) : v(il) {}
  Err3(std::initializer_list<int> il, const Err& err_) : v(il), err(err_) {}

  Err3(std::initializer_list<int> il, int e_, Err&& err_)
      : v(il), e(e_), err(std::move(err_)) {}

  std::vector<int> v;
  int e = 0;
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
  // (std::in_place_t, Args&&...)
  {
    unexpected<Err3> e(std::in_place);
    ASSERT_EQ(e.value().e, 0);
    ASSERT_EQ(e.value().err.e, 0);
  }
  {
    int val = 7;
    unexpected<Err3> e(std::in_place, val);
    ASSERT_EQ(e.value().e, 7);
    ASSERT_EQ(e.value().err.e, 0);
    ASSERT_EQ(val, 7);
  }
  {
    Err val(9);
    unexpected<Err3> e(std::in_place, 8, val);
    ASSERT_EQ(e.value().e, 8);
    ASSERT_EQ(e.value().err.e, 9);
    ASSERT_EQ(val.e, 9);
  }
  {
    Err val(11);
    unexpected<Err3> e(std::in_place, 10, std::move(val));
    ASSERT_EQ(e.value().e, 10);
    ASSERT_EQ(e.value().err.e, 11);
    ASSERT_EQ(val.e, -1);
  }
  // (std::in_place_t, std::initializer_list<U>, Args&&...)
  {
    unexpected<Err3> e(std::in_place, {12, 13, 14});
    ASSERT_EQ(e.value().v.size(), 3);
    ASSERT_EQ(e.value().v[0], 12);
    ASSERT_EQ(e.value().v[1], 13);
    ASSERT_EQ(e.value().v[2], 14);
  }
  {
    Err val(17);
    unexpected<Err3> e(std::in_place, {15, 16}, val);
    ASSERT_EQ(e.value().v.size(), 2);
    ASSERT_EQ(e.value().v[0], 15);
    ASSERT_EQ(e.value().v[1], 16);
    ASSERT_EQ(e.value().e, 0);
    ASSERT_EQ(e.value().err.e, 17);
    ASSERT_EQ(val.e, 17);
  }
  {
    Err val(20);
    unexpected<Err3> e(std::in_place, {18}, 19, std::move(val));
    ASSERT_EQ(e.value().v.size(), 1);
    ASSERT_EQ(e.value().v[0], 18);
    ASSERT_EQ(e.value().e, 19);
    ASSERT_EQ(e.value().err.e, 20);
    ASSERT_EQ(val.e, -1);
  }
}

TEST(unexpected, equality_operators) {
  unexpected<Err> e_one(1);
  unexpected<Err> e1(1);
  unexpected<Err> e2(2);

  ASSERT_TRUE(e_one == e1);
  ASSERT_FALSE(e_one == e2);

  ASSERT_FALSE(e_one != e1);
  ASSERT_TRUE(e_one != e2);

  unexpected<Err2> ee1(Err(1));
  unexpected<Err2> ee2(Err(2));

  ASSERT_TRUE(e_one == ee1);
  ASSERT_FALSE(e_one == ee2);

  ASSERT_FALSE(e_one != ee1);
  ASSERT_TRUE(e_one != ee2);
}
