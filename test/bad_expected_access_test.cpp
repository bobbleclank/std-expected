#include "bc/expected.h"

#include "obj.h"

#include <utility>

#include <gtest/gtest.h>

using namespace bc;

TEST(bad_expected_access, what) {
  bad_expected_access<Err> e(Err(1));
  ASSERT_STREQ(e.what(), "bad expected access");
}

TEST(bad_expected_access, error) {
  // const& overload
  {
    const bad_expected_access<Err> e(Err(1));
    const Err& err = e.error();
    ASSERT_EQ(err.x, 1);
    ASSERT_EQ(e.error().x, 1);
  }
  // non-const& overload
  {
    bad_expected_access<Err> e(Err(2));
    Err& err = e.error();
    ASSERT_EQ(err.x, 2);
    ASSERT_EQ(e.error().x, 2);
    err.x = 20;
    ASSERT_EQ(err.x, 20);
    ASSERT_EQ(e.error().x, 20);
  }
  // const&& overload
  {
    const bad_expected_access<Err> e(Err(3));
    Err err = std::move(e).error();
    ASSERT_EQ(err.x, 3);
    ASSERT_EQ(e.error().x, 3);
    // Since the r-value reference is const, Err's copy constructor is called,
    // and not Err's move constructor (which takes a non-const r-value
    // reference).
  }
  {
    const bad_expected_access<Err> e(Err(4));
    Err err(40);
    err = std::move(e).error();
    ASSERT_EQ(err.x, 4);
    ASSERT_EQ(e.error().x, 4);
    // Since the r-value reference is const, Err's copy assignment is called,
    // and not Err's move assignment (which takes a non-const r-value
    // reference).
  }
  // non-const&& overload
  {
    bad_expected_access<Err> e(Err(5));
    Err err = std::move(e).error();
    ASSERT_EQ(err.x, 5);
    ASSERT_EQ(e.error().x, -1);
  }
  {
    bad_expected_access<Err> e(Err(6));
    Err err(60);
    err = std::move(e).error();
    ASSERT_EQ(err.x, 6);
    ASSERT_EQ(e.error().x, -2);
  }
}

TEST(bad_expected_access, constructor) {
  {
    Err err(1);
    bad_expected_access<Err> e(err);
    ASSERT_EQ(e.error().x, 1);
    ASSERT_EQ(err.x, 1);
  }
  {
    Err err(2);
    bad_expected_access<Err> e(std::move(err));
    ASSERT_EQ(e.error().x, 2);
    ASSERT_EQ(err.x, -1);
  }
}
