#ifndef TEST_OBJ_CONSTEXPR_H
#define TEST_OBJ_CONSTEXPR_H

#include <initializer_list>
#include <utility>

struct Arg {
  constexpr explicit Arg(int x_) : x(x_) {}

  Arg(const Arg&) = default;

  constexpr Arg(Arg&& other) noexcept : x(other.x + 201) {}

  Arg& operator=(const Arg&) = default;

  constexpr Arg& operator=(Arg&& other) noexcept {
    x = other.x + 202;
    return *this;
  }

  ~Arg() = default;

  int x;
};

template <class Tag>
struct Obj {
  constexpr Obj() noexcept : x(20100) {}

  constexpr explicit Obj(int x_) noexcept : x(x_) {}

  constexpr explicit Obj(const Arg& arg_) noexcept : x(arg_.x) {}

  constexpr explicit Obj(Arg&& arg_) noexcept {
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  constexpr Obj(Arg&& arg_, int i) noexcept {
    Arg arg = std::move(arg_);
    x = arg.x + i;
  }

  constexpr Obj(std::initializer_list<int> il, Arg&& arg_, int i) noexcept {
    Arg arg = std::move(arg_);
    x = arg.x + i;
    if (!std::empty(il))
      x += *il.begin();
  }

  Obj(const Obj&) = default;

  constexpr Obj(Obj&& other) noexcept : x(other.x + 101) {}

  Obj& operator=(const Obj&) = default;

  constexpr Obj& operator=(Obj&& other) noexcept {
    x = other.x + 102;
    return *this;
  }

  constexpr Obj& operator=(const Arg& arg_) noexcept {
    x = arg_.x;
    return *this;
  }

  constexpr Obj& operator=(Arg&& arg_) noexcept {
    Arg arg = std::move(arg_);
    x = arg.x;
    return *this;
  }

  ~Obj() = default;

  int x;
};

template <class Tag1, class Tag2>
constexpr bool operator==(Obj<Tag1> lhs, Obj<Tag2> rhs) {
  return lhs.x == rhs.x;
}

template <class Tag1, class Tag2>
constexpr bool operator!=(Obj<Tag1> lhs, Obj<Tag2> rhs) {
  return !(lhs == rhs);
}

struct Val_tag {};
using Val = Obj<Val_tag>;

struct Err_tag {};
using Err = Obj<Err_tag>;

template <class Tag>
struct Obj_implicit {
  constexpr Obj_implicit(int x_) : x(x_) {}

  constexpr Obj_implicit(const Arg& arg_) : x(arg_.x) {}

  constexpr Obj_implicit(Arg&& arg_) {
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  Obj_implicit(const Obj_implicit&) = delete;

  Obj_implicit(Obj_implicit&&) = delete;

  Obj_implicit& operator=(const Obj_implicit&) = delete;

  Obj_implicit& operator=(Obj_implicit&&) = delete;

  ~Obj_implicit() = default;

  int x;
};

struct Val_implicit_tag {};
using Val_implicit = Obj_implicit<Val_implicit_tag>;

struct Err_implicit_tag {};
using Err_implicit = Obj_implicit<Err_implicit_tag>;

#endif
