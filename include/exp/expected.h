#ifndef INCLUDE_EXP_EXPECTED_H
#define INCLUDE_EXP_EXPECTED_H

#include <exception>
#include <initializer_list>
#include <type_traits>
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
  static_assert(!std::is_same_v<E, void>);
  static_assert(!std::is_reference_v<E>);

  unexpected() = delete;

  constexpr unexpected(const unexpected&) = default;
  constexpr unexpected(unexpected&&) = default;

  template <class Err = E,
            std::enable_if_t<
                std::is_constructible_v<E, Err&&> &&
                !std::is_same_v<std::decay_t<Err>, std::in_place_t> &&
                !std::is_same_v<std::decay_t<Err>, unexpected<E>>>* = nullptr>
  constexpr explicit unexpected(Err&& val) : val_(std::forward<Err>(val)) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr explicit unexpected(std::in_place_t, Args&&... args)
      : val_(std::forward<Args>(args)...) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                E, std::initializer_list<U>, Args&&...>>* = nullptr>
  constexpr explicit unexpected(std::in_place_t, std::initializer_list<U> il,
                                Args&&... args)
      : val_(il, std::forward<Args>(args)...) {}

  ~unexpected() = default;

  constexpr unexpected& operator=(const unexpected&) = default;
  constexpr unexpected& operator=(unexpected&&) = default;

  constexpr const E& value() const& noexcept { return val_; }
  constexpr E& value() & noexcept { return val_; }
  constexpr const E&& value() const&& noexcept { return std::move(val_); }
  constexpr E&& value() && noexcept { return std::move(val_); }

private:
  E val_;
};

template <class E> unexpected(E) -> unexpected<E>;

template <class E1, class E2>
constexpr bool operator==(const unexpected<E1>& x, const unexpected<E2>& y) {
  return x.value() == y.value();
}

template <class E1, class E2>
constexpr bool operator!=(const unexpected<E1>& x, const unexpected<E2>& y) {
  return x.value() != y.value();
}

template <class T, class E> class expected {
public:
private:
};

} // namespace exp

#endif
