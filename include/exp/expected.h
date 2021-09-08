#ifndef INCLUDE_EXP_EXPECTED_H
#define INCLUDE_EXP_EXPECTED_H

#include <exception>
#include <utility>

namespace exp {

struct unexpect_t {
  explicit unexpect_t() = default;
};

inline constexpr unexpect_t unexpect{};

template <class E> class bad_expected_access;

template <> class bad_expected_access<void> : public std::exception {
public:
  explicit bad_expected_access() = default;
};

template <class E>
class bad_expected_access : public bad_expected_access<void> {
public:
  explicit bad_expected_access(E val) : val_(std::move(val)) {}

  const char* what() const noexcept override { return "bad expected access"; }

  const E& error() const& { return val_; }
  E& error() & { return val_; }
  const E&& error() const&& { return std::move(val_); }
  E&& error() && { return std::move(val_); }

private:
  E val_;
};

template <class E> class unexpected {
public:
private:
};

template <class T, class E> class expected {
public:
private:
};

} // namespace exp

#endif
