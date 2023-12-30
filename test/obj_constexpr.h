#ifndef TEST_OBJ_CONSTEXPR_H
#define TEST_OBJ_CONSTEXPR_H

#include <initializer_list>
#include <iterator>
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
  Obj() = default;

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
    Arg arg(0);
    arg = std::move(arg_);
    x = arg.x;
    return *this;
  }

  ~Obj() = default;

  int x = 20100;
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
  constexpr explicit Obj_implicit(int x_) : x(x_) {}

  constexpr Obj_implicit(const Arg& arg_) : x(arg_.x) {}

  constexpr Obj_implicit(Arg&& arg_) {
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  Obj_implicit(const Obj_implicit&) = default;

  constexpr Obj_implicit(Obj_implicit&& other) : x(other.x + 301) {}

  Obj_implicit& operator=(const Obj_implicit&) = delete;

  Obj_implicit& operator=(Obj_implicit&&) = delete;

  ~Obj_implicit() = default;

  int x;
};

struct Val_implicit_tag {};
using Val_implicit = Obj_implicit<Val_implicit_tag>;

struct Err_implicit_tag {};
using Err_implicit = Obj_implicit<Err_implicit_tag>;

template <class Tag>
struct Obj_explicit {
  constexpr explicit Obj_explicit(int x_) : x(x_) {}

  constexpr explicit Obj_explicit(const Arg& arg_) : x(arg_.x) {}

  constexpr explicit Obj_explicit(Arg&& arg_) {
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  constexpr explicit Obj_explicit(const Obj_explicit& other) : x(other.x) {}

  constexpr explicit Obj_explicit(Obj_explicit&& other) : x(other.x + 401) {}

  Obj_explicit& operator=(const Obj_explicit&) = delete;

  Obj_explicit& operator=(Obj_explicit&&) = delete;

  ~Obj_explicit() = default;

  int x;
};

struct Val_explicit_tag {};
using Val_explicit = Obj_explicit<Val_explicit_tag>;

struct Err_explicit_tag {};
using Err_explicit = Obj_explicit<Err_explicit_tag>;

template <class Tag>
struct Obj_trivial {
  constexpr explicit Obj_trivial(int x_) : x(x_) {}

  Obj_trivial(const Obj_trivial&) = default;

  Obj_trivial(Obj_trivial&&) = default;

  Obj_trivial& operator=(const Obj_trivial&) = default;

  Obj_trivial& operator=(Obj_trivial&&) = default;

  ~Obj_trivial() = default;

  int x;
};

struct Val_trivial_tag {};
using Val_trivial = Obj_trivial<Val_trivial_tag>;

struct Err_trivial_tag {};
using Err_trivial = Obj_trivial<Err_trivial_tag>;

#endif
