#ifndef TEST_OBJ_H
#define TEST_OBJ_H

#include "arg.h"
#include "state.h"

#include <initializer_list>
#include <utility>

template <class Tag> struct Obj {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  Obj() { s = State::default_constructed; }

  explicit Obj(int x_) {
    s = State::constructed;
    x = x_;
  }

  Obj(Arg&& arg_, int) noexcept {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  Obj(std::initializer_list<int> il, Arg&& arg_, int) noexcept {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
    if (!std::empty(il))
      x += *il.begin();
  }

  Obj(const Obj& other) noexcept {
    s = State::copy_constructed;
    x = other.x;
  }

  Obj(Obj&& other) noexcept {
    s = State::move_constructed;
    x = other.x;
    other.x = -1;
  }

  Obj& operator=(const Obj& other) noexcept {
    s = State::copy_assigned;
    x = other.x;
    return *this;
  }

  Obj& operator=(Obj&& other) noexcept {
    s = State::move_assigned;
    x = other.x;
    other.x = -2;
    return *this;
  }

  ~Obj() { s = State::destructed; }

  int x = 0;
};

template <class Tag> bool operator==(Obj<Tag> lhs, Obj<Tag> rhs) {
  return lhs.x == rhs.x;
}

template <class Tag> bool operator!=(Obj<Tag> lhs, Obj<Tag> rhs) {
  return !(lhs == rhs);
}

struct Val_tag {};
using Val = Obj<Val_tag>;

struct Err_tag {};
using Err = Obj<Err_tag>;

#endif
