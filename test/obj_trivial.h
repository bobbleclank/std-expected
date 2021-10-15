#ifndef TEST_OBJ_TRIVIAL_H
#define TEST_OBJ_TRIVIAL_H

#include "arg.h"

#include <initializer_list>
#include <utility>

template <class Tag> struct Obj_trivial {
  Obj_trivial() = default;

  Obj_trivial(Arg&& arg_, int) {
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  Obj_trivial(std::initializer_list<int> il, Arg&& arg_, int) {
    Arg arg = std::move(arg_);
    x = arg.x;
    if (!std::empty(il))
      x += *il.begin();
  }

  ~Obj_trivial() = default;

  int x = 20100;
};

#endif
