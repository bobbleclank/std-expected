#ifndef TEST_OBJ_TRIVIAL_H
#define TEST_OBJ_TRIVIAL_H

#include "arg.h"

#include <initializer_list>
#include <utility>

template <class Tag> struct Obj_trivial {
  Obj_trivial() = default;

  Obj_trivial(Arg&& arg_, int i) {
    Arg arg = std::move(arg_);
    x = arg.x + i;
  }

  Obj_trivial(std::initializer_list<int> il, Arg&& arg_, int i) {
    Arg arg = std::move(arg_);
    x = arg.x + i;
    if (!std::empty(il))
      x += *il.begin();
  }

  Obj_trivial(const Obj_trivial&) = default;

  Obj_trivial(Obj_trivial&&) = default;

  Obj_trivial& operator=(const Obj_trivial&) = default;

  Obj_trivial& operator=(Obj_trivial&&) = default;

  ~Obj_trivial() = default;

  int x = 20100;
};

struct Val_trivial_tag {};
using Val_trivial = Obj_trivial<Val_trivial_tag>;

struct Err_trivial_tag {};
using Err_trivial = Obj_trivial<Err_trivial_tag>;

#endif
