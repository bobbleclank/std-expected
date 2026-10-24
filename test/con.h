#ifndef TEST_CON_H
#define TEST_CON_H

#include "obj.h"

struct Con {
  explicit Con(int x_) : x(x_) {}

  // NOLINTNEXTLINE(*-explicit-constructor): To test implicit conversion
  operator Val() const& { return Val(x); }

  // NOLINTNEXTLINE(*-explicit-constructor): To test implicit conversion
  operator Val() && {
    Val val(x);
    x = -3;
    return val;
  }

  int x;
};

#endif
