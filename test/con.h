#ifndef TEST_CON_H
#define TEST_CON_H

#include "obj.h"

struct Con {
  explicit Con(int x_) : x(x_) {}

  operator Val() const& { return Val(x); }

  operator Val() && {
    Val val(x);
    x = -3;
    return val;
  }

  int x = 20100;
};

#endif
