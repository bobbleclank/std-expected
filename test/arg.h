#ifndef TEST_ARG_H
#define TEST_ARG_H

struct Arg {
  Arg() = default;
  explicit Arg(int x_) : x(x_) {}
  ~Arg() = default;

  Arg(const Arg&) = default;
  Arg& operator=(const Arg&) = default;

  Arg(Arg&& other) noexcept {
    x = other.x;
    other.x = -1;
  }

  Arg& operator=(Arg&& other) noexcept {
    x = other.x;
    other.x = -2;
    return *this;
  }

  int x = 0;
};

#endif
