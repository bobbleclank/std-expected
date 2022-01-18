#ifndef TEST_ERR_H
#define TEST_ERR_H

#include "arg.h"

#include <initializer_list>
#include <utility>

struct Err {
  Err() = default;

  explicit Err(int x_) : x(x_) {}

  Err(Arg&& arg_, int) {
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  Err(std::initializer_list<int> il, Arg&& arg_, int) {
    Arg arg = std::move(arg_);
    x = arg.x;
    if (!std::empty(il))
      x += *il.begin();
  }

  Err(const Err&) = default;

  Err(Err&& other) noexcept {
    x = other.x;
    other.x = -1;
  }

  Err& operator=(const Err&) = default;

  Err& operator=(Err&& other) noexcept {
    x = other.x;
    other.x = -2;
    return *this;
  }

  ~Err() = default;

  int x = 20100;
};

inline bool operator==(Err lhs, Err rhs) { return lhs.x == rhs.x; }
inline bool operator!=(Err lhs, Err rhs) { return !(lhs == rhs); }

#endif
