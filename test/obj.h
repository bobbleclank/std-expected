#ifndef TEST_OBJ_H
#define TEST_OBJ_H

#include "arg.h"
#include "state.h"

#include <initializer_list>
#include <utility>

template <class Tag> struct Obj {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  Obj() noexcept { s = State::default_constructed; }

  explicit Obj(int x_) noexcept {
    s = State::constructed;
    x = x_;
  }

  explicit Obj(const Arg& arg_) noexcept {
    s = State::constructed;
    Arg arg = arg_;
    x = arg.x;
  }

  explicit Obj(Arg&& arg_) noexcept {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  Obj(Arg&& arg_, int i) noexcept {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x + i;
  }

  Obj(std::initializer_list<int> il, Arg&& arg_, int i) noexcept {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x + i;
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

  Obj& operator=(const Arg& arg_) noexcept {
    s = State::assigned;
    Arg arg = arg_;
    x = arg.x;
    return *this;
  }

  Obj& operator=(Arg&& arg_) noexcept {
    s = State::assigned;
    Arg arg = std::move(arg_);
    x = arg.x;
    return *this;
  }

  ~Obj() { s = State::destructed; }

  int x = 20100;
};

template <class Tag1, class Tag2>
bool operator==(Obj<Tag1> lhs, Obj<Tag2> rhs) {
  return lhs.x == rhs.x;
}

template <class Tag1, class Tag2>
bool operator!=(Obj<Tag1> lhs, Obj<Tag2> rhs) {
  return !(lhs == rhs);
}

struct Val_tag {};
using Val = Obj<Val_tag>;

struct Err_tag {};
using Err = Obj<Err_tag>;

struct Val2_tag {};
using Val2 = Obj<Val2_tag>;

struct Err2_tag {};
using Err2 = Obj<Err2_tag>;

#endif
