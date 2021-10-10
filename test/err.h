#ifndef TEST_ERR_H
#define TEST_ERR_H

#include "arg.h"

#include <initializer_list>
#include <utility>

struct Err {
  Err() = default;
  explicit Err(int e_) : e(e_) {}
  ~Err() = default;

  Err(Arg&& arg_, int) {
    Arg arg = std::move(arg_);
    e = arg.x;
  }

  Err(std::initializer_list<int> il, Arg&& arg_, int) {
    Arg arg = std::move(arg_);
    e = arg.x;
    if (!std::empty(il))
      e += *il.begin();
  }

  Err(const Err&) = default;
  Err& operator=(const Err&) = default;

  Err(Err&& other) noexcept {
    e = other.e;
    other.e = -1;
  }

  Err& operator=(Err&& other) noexcept {
    e = other.e;
    other.e = -2;
    return *this;
  }

  int e = 0;
};

inline bool operator==(Err lhs, Err rhs) { return lhs.e == rhs.e; }
inline bool operator!=(Err lhs, Err rhs) { return !(lhs == rhs); }

#endif
