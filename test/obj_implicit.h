#ifndef TEST_OBJ_IMPLICIT_H
#define TEST_OBJ_IMPLICIT_H

#include "arg.h"
#include "state.h"

#include <utility>

template <class Tag> struct Obj_implicit {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  Obj_implicit(int x_) {
    s = State::constructed;
    x = x_;
  }

  Obj_implicit(const Arg& arg_) {
    s = State::constructed;
    Arg arg = arg_;
    x = arg.x;
  }

  Obj_implicit(Arg&& arg_) {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  Obj_implicit(const Obj_implicit& other) {
    s = State::copy_constructed;
    x = other.x;
  }

  Obj_implicit(Obj_implicit&& other) {
    s = State::move_constructed;
    x = other.x;
    other.x = -1;
  }

  Obj_implicit& operator=(const Obj_implicit&) = delete;

  Obj_implicit& operator=(Obj_implicit&& other) = delete;

  ~Obj_implicit() { s = State::destructed; }

  int x = 20100;
};

struct Val_implicit_tag {};
using Val_implicit = Obj_implicit<Val_implicit_tag>;

struct Err_implicit_tag {};
using Err_implicit = Obj_implicit<Err_implicit_tag>;

#endif
