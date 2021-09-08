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

} // namespace

TEST(bad_expected_access, what) {
  bad_expected_access<int> e(1);
  ASSERT_EQ(e.what(), "bad expected access");
}

TEST(bad_expected_access, error) {
  // const& overload
  {
    const bad_expected_access<Err> e(Err(1));
    const Err& err = e.error();
    ASSERT_EQ(err.e, 1);
    ASSERT_EQ(e.error().e, 1);
  }
  // non-const& overload
  {
    bad_expected_access<Err> e(Err(2));
    Err& err = e.error();
    ASSERT_EQ(err.e, 2);
    ASSERT_EQ(e.error().e, 2);
    err.e = 20;
    ASSERT_EQ(err.e, 20);
    ASSERT_EQ(e.error().e, 20);
  }
  // const&& overload
  {
    const bad_expected_access<Err> e(Err(3));
    Err err = std::move(e).error();
    ASSERT_EQ(err.e, 3);
    ASSERT_EQ(e.error().e, 3);
    // Is this correct? I need to better understand const&&.
  }
  {
    const bad_expected_access<Err> e(Err(4));
    Err err;
    err = std::move(e).error();
    ASSERT_EQ(err.e, 4);
    ASSERT_EQ(e.error().e, 4);
    // Is this correct? I need to better understand const&&.
  }
  // non-const&& overload
  {
    bad_expected_access<Err> e(Err(5));
    Err err = std::move(e).error();
    ASSERT_EQ(err.e, 5);
    ASSERT_EQ(e.error().e, -1);
  }
  {
    bad_expected_access<Err> e(Err(6));
    Err err;
    err = std::move(e).error();
    ASSERT_EQ(err.e, 6);
    ASSERT_EQ(e.error().e, -2);
  }
}

TEST(bad_expected_access, constructor) {
  {
    Err err(1);
    bad_expected_access<Err> e(err);
    ASSERT_EQ(e.error().e, 1);
    ASSERT_EQ(err.e, 1);
  }
  {
    Err err(2);
    bad_expected_access<Err> e(std::move(err));
    ASSERT_EQ(e.error().e, 2);
    ASSERT_EQ(err.e, -1);
  }
}
