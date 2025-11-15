#ifndef TEST_OBJ_THROW_H
#define TEST_OBJ_THROW_H

#include "arg.h"
#include "state.h"

#include <initializer_list>
#include <iterator>
#include <utility>

enum class May_throw {
  do_not_throw,
  do_throw
};

template <class Tag>
struct Obj_throw {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  explicit Obj_throw(int x_) : x(x_) { s = State::constructed; }

  Obj_throw(Arg&& arg_, int i) {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x + i;
    if (t == May_throw::do_throw)
      throw t;
  }

  Obj_throw(std::initializer_list<int> il, Arg&& arg_, int i) {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x + i;
    if (!std::empty(il))
      x += *il.begin();
    if (t == May_throw::do_throw)
      throw t;
  }

  Obj_throw(const Obj_throw& other) : x(other.x) {
    s = State::copy_constructed;
    if (t == May_throw::do_throw)
      throw t;
  }

  Obj_throw(Obj_throw&& other) noexcept : x(other.x) {
    s = State::move_constructed;
    other.x = -1;
  }

  Obj_throw& operator=(const Obj_throw& other) {
    s = State::copy_assigned;
    x = other.x;
    if (t == May_throw::do_throw)
      throw t;
    return *this;
  }

  Obj_throw& operator=(Obj_throw&& other) noexcept {
    s = State::move_assigned;
    x = other.x;
    other.x = -2;
    return *this;
  }

  ~Obj_throw() { s = State::destructed; }

  int x;
  inline static May_throw t = May_throw::do_not_throw;
};

struct Val_throw_tag {};
using Val_throw = Obj_throw<Val_throw_tag>;

struct Err_throw_tag {};
using Err_throw = Obj_throw<Err_throw_tag>;

template <class Tag>
struct Obj_throw_2 {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  explicit Obj_throw_2(int x_) : x(x_) { s = State::constructed; }

  explicit Obj_throw_2(const Arg& arg_) {
    s = State::constructed;
    Arg arg = arg_;
    x = arg.x;
    if (t == May_throw::do_throw)
      throw t;
  }

  explicit Obj_throw_2(Arg&& arg_) {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
    if (t == May_throw::do_throw)
      throw t;
  }

  Obj_throw_2(Arg&& arg_, int i) {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x + i;
    if (t == May_throw::do_throw)
      throw t;
  }

  Obj_throw_2(std::initializer_list<int> il, Arg&& arg_, int i) {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x + i;
    if (!std::empty(il))
      x += *il.begin();
    if (t == May_throw::do_throw)
      throw t;
  }

  Obj_throw_2(const Obj_throw_2& other) : x(other.x) {
    s = State::copy_constructed;
    if (t == May_throw::do_throw)
      throw t;
  }

  // NOLINTNEXTLINE(*-noexcept-move-operations): May throw
  Obj_throw_2(Obj_throw_2&& other) : x(other.x) {
    s = State::move_constructed;
    other.x = -1;
    if (t == May_throw::do_throw)
      throw t;
  }

  Obj_throw_2& operator=(const Obj_throw_2& other) {
    s = State::copy_assigned;
    x = other.x;
    if (t == May_throw::do_throw)
      throw t;
    return *this;
  }

  // NOLINTNEXTLINE(*-noexcept-move-operations): May throw
  Obj_throw_2& operator=(Obj_throw_2&& other) {
    s = State::move_assigned;
    x = other.x;
    other.x = -2;
    if (t == May_throw::do_throw)
      throw t;
    return *this;
  }

  Obj_throw_2& operator=(const Arg& arg_) {
    s = State::assigned;
    Arg arg = arg_;
    x = arg.x;
    if (t == May_throw::do_throw)
      throw t;
    return *this;
  }

  Obj_throw_2& operator=(Arg&& arg_) {
    s = State::assigned;
    Arg arg = std::move(arg_);
    x = arg.x;
    if (t == May_throw::do_throw)
      throw t;
    return *this;
  }

  ~Obj_throw_2() { s = State::destructed; }

  int x;
  inline static May_throw t = May_throw::do_not_throw;
};

struct Val_throw_2_tag {};
using Val_throw_2 = Obj_throw_2<Val_throw_2_tag>;

struct Err_throw_2_tag {};
using Err_throw_2 = Obj_throw_2<Err_throw_2_tag>;

template <class Tag>
struct Obj_throw_3 {
  inline static State s = State::none;
  static void reset() { s = State::none; }

  explicit Obj_throw_3(int x_) : x(x_) { s = State::constructed; }

  explicit Obj_throw_3(const Arg& arg_) noexcept {
    s = State::constructed;
    Arg arg = arg_;
    x = arg.x;
  }

  explicit Obj_throw_3(Arg&& arg_) noexcept {
    s = State::constructed;
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  Obj_throw_3(const Obj_throw_3& other) noexcept : x(other.x) {
    s = State::copy_constructed;
  }

  Obj_throw_3(Obj_throw_3&& other) noexcept : x(other.x) {
    s = State::move_constructed;
    other.x = -1;
  }

  Obj_throw_3& operator=(const Obj_throw_3& other) {
    s = State::copy_assigned;
    x = other.x;
    if (t == May_throw::do_throw)
      throw t;
    return *this;
  }

  // NOLINTNEXTLINE(*-noexcept-move-operations): May throw
  Obj_throw_3& operator=(Obj_throw_3&& other) {
    s = State::move_assigned;
    x = other.x;
    other.x = -2;
    if (t == May_throw::do_throw)
      throw t;
    return *this;
  }

  Obj_throw_3& operator=(const Arg& arg_) {
    s = State::assigned;
    Arg arg = arg_;
    x = arg.x;
    if (t == May_throw::do_throw)
      throw t;
    return *this;
  }

  Obj_throw_3& operator=(Arg&& arg_) {
    s = State::assigned;
    Arg arg = std::move(arg_);
    x = arg.x;
    if (t == May_throw::do_throw)
      throw t;
    return *this;
  }

  ~Obj_throw_3() { s = State::destructed; }

  int x;
  inline static May_throw t = May_throw::do_not_throw;
};

struct Err_throw_3_tag {};
using Err_throw_3 = Obj_throw_3<Err_throw_3_tag>;

#endif
