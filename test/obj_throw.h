#ifndef TEST_OBJ_THROW_H
#define TEST_OBJ_THROW_H

#include "arg.h"
#include "state.h"

#include <initializer_list>
#include <utility>

enum class May_throw { do_not_throw, do_throw };

struct Val_throw {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  explicit Val_throw(int x_) {
    s = State::constructed;
    x = x_;
  }

  Val_throw(Arg&& arg_, int) {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
    if (t == May_throw::do_throw)
      throw t;
  }

  Val_throw(std::initializer_list<int> il, Arg&& arg_, int) {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
    if (!std::empty(il))
      x += *il.begin();
    if (t == May_throw::do_throw)
      throw t;
  }

  Val_throw(Val_throw&& other) noexcept {
    s = State::move_constructed;
    x = other.x;
    other.x = -1;
  }

  Val_throw& operator=(Val_throw&& other) noexcept {
    s = State::move_assigned;
    x = other.x;
    other.x = -2;
    return *this;
  }

  ~Val_throw() { s = State::destructed; }

  int x = 0;
  inline static May_throw t = May_throw::do_not_throw;
};

struct Val_throw_2 {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  Val_throw_2(Arg&& arg_, int) {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
    if (t == May_throw::do_throw)
      throw t;
  }

  Val_throw_2(std::initializer_list<int> il, Arg&& arg_, int) {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
    if (!std::empty(il))
      x += *il.begin();
    if (t == May_throw::do_throw)
      throw t;
  }

  Val_throw_2(Val_throw_2&& other) {
    s = State::move_constructed;
    x = other.x;
    other.x = -1;
  }

  Val_throw_2& operator=(Val_throw_2&& other) {
    s = State::move_assigned;
    x = other.x;
    other.x = -2;
    return *this;
  }

  ~Val_throw_2() { s = State::destructed; }

  int x = 0;
  inline static May_throw t = May_throw::do_not_throw;
};

#endif
