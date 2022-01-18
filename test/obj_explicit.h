#ifndef TEST_OBJ_EXPLICIT_H
#define TEST_OBJ_EXPLICIT_H

#include "arg.h"
#include "state.h"

#include <utility>

template <class Tag> struct Obj_explicit {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  explicit Obj_explicit(int x_) {
    s = State::constructed;
    x = x_;
  }

  explicit Obj_explicit(const Arg& arg_) {
    s = State::constructed;
    Arg arg = arg_;
    x = arg.x;
  }

  explicit Obj_explicit(Arg&& arg_) {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  explicit Obj_explicit(const Obj_explicit& other) {
    s = State::copy_constructed;
    x = other.x;
  }

  explicit Obj_explicit(Obj_explicit&& other) {
    s = State::move_constructed;
    x = other.x;
    other.x = -1;
  }

  ~Obj_explicit() { s = State::destructed; }

  int x = 20100;
};

struct Val_explicit_tag {};
using Val_explicit = Obj_explicit<Val_explicit_tag>;

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

  ~Obj_implicit() { s = State::destructed; }

  int x = 20100;
};

struct Val_implicit_tag {};
using Val_implicit = Obj_implicit<Val_implicit_tag>;

#endif
