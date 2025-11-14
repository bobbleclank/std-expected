#ifndef TEST_ARG_H
#define TEST_ARG_H

struct Arg {
  explicit Arg(int x_) : x(x_) {}

  Arg(const Arg&) = default;

  Arg(Arg&& other) noexcept : x(other.x) { other.x = -1; }

  Arg& operator=(const Arg&) = default;

  Arg& operator=(Arg&& other) noexcept {
    x = other.x;
    other.x = -2;
    return *this;
  }

  ~Arg() = default;

  int x;
};

#endif
