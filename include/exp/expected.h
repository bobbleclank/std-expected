#ifndef INCLUDE_EXP_EXPECTED_H
#define INCLUDE_EXP_EXPECTED_H

#include <exception>
#include <initializer_list>
#include <memory>
#include <optional>
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

  virtual const char* what() const noexcept override {
    return "bad expected access";
  }

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

  // template <class Err>
  // constexpr explicit(see below) unexpected(const unexpected<Err>& other);

  // template <class Err>
  // constexpr explicit(see below) unexpected(unexpected<Err>&& other);

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

  // template <class Err = E>
  // constexpr unexpected& operator=(const unexpected<Err>& other);

  // template <class Err = E>
  // constexpr unexpected& operator=(unexpected<Err>&& other);

  constexpr const E& value() const& noexcept { return val_; }
  constexpr E& value() & noexcept { return val_; }
  constexpr const E&& value() const&& noexcept { return std::move(val_); }
  constexpr E&& value() && noexcept { return std::move(val_); }

  // void swap(unexpected& other) noexcept(see below);

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

// template <class E>
// void swap(unexpected<E>& x, unexpected<E>& y) noexcept(noexcept(x.swap(y)));

template <class T, class E> class expected {
public:
  static_assert(!std::is_same_v<T, std::remove_cv_t<unexpected<E>>>);
  static_assert(!std::is_same_v<T, std::remove_cv_t<std::in_place_t>>);
  static_assert(!std::is_same_v<T, std::remove_cv_t<unexpect_t>>);
  static_assert(!std::is_reference_v<T>);

  static_assert(!std::is_same_v<E, void>);
  static_assert(!std::is_reference_v<E>);

  using value_type = T;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  template <class U> using rebind = expected<U, error_type>;

  constexpr expected() : val_(std::in_place), has_val_(true) {}

  constexpr expected(const expected&) = default;
  constexpr expected(expected&&) = default;

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<T, Args&&...>>* = nullptr>
  constexpr explicit expected(std::in_place_t, Args&&... args)
      : val_(std::in_place, std::forward<Args>(args)...), has_val_(true) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                T, std::initializer_list<U>, Args&&...>>* = nullptr>
  constexpr explicit expected(std::in_place_t, std::initializer_list<U> il,
                              Args&&... args)
      : val_(std::in_place, il, std::forward<Args>(args)...), has_val_(true) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr explicit expected(unexpect_t, Args&&... args)
      : unexpect_(std::in_place, std::in_place, std::forward<Args>(args)...),
        has_val_(false) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                E, std::initializer_list<U>, Args&&...>>* = nullptr>
  constexpr explicit expected(unexpect_t, std::initializer_list<U> il,
                              Args&&... args)
      : unexpect_(std::in_place, std::in_place, il,
                  std::forward<Args>(args)...),
        has_val_(false) {}

  ~expected() = default;

  expected& operator=(const expected&) = default;
  expected& operator=(expected&&) = default;

  constexpr const T* operator->() const { return std::addressof(*val_); }
  constexpr T* operator->() { return std::addressof(*val_); }

  constexpr const T& operator*() const& { return *val_; }
  constexpr T& operator*() & { return *val_; }
  constexpr const T&& operator*() const&& { return std::move(*val_); }
  constexpr T&& operator*() && { return std::move(*val_); }

  constexpr explicit operator bool() const noexcept { return has_val_; }
  constexpr bool has_value() const noexcept { return has_val_; }

  constexpr const T& value() const& {
    if (!has_val_)
      throw bad_expected_access(unexpect_->value());
    return *val_;
  }

  constexpr T& value() & {
    if (!has_val_)
      throw bad_expected_access(unexpect_->value());
    return *val_;
  }

  constexpr const T&& value() const&& {
    if (!has_val_)
      throw bad_expected_access(std::move(unexpect_->value()));
    return std::move(*val_);
  }

  constexpr T&& value() && {
    if (!has_val_)
      throw bad_expected_access(std::move(unexpect_->value()));
    return std::move(*val_);
  }

  constexpr const E& error() const& { return unexpect_->value(); }
  constexpr E& error() & { return unexpect_->value(); }
  constexpr const E&& error() const&& { return std::move(unexpect_->value()); }
  constexpr E&& error() && { return std::move(unexpect_->value()); }

  template <class U> constexpr T value_or(U&& v) const& {
    static_assert(std::is_copy_constructible_v<T> &&
                  std::is_convertible_v<U&&, T>);
    return has_val_ ? *val_ : static_cast<T>(std::forward<U>(v));
  }

  template <class U> constexpr T value_or(U&& v) && {
    static_assert(std::is_move_constructible_v<T> &&
                  std::is_convertible_v<U&&, T>);
    return has_val_ ? std::move(*val_) : static_cast<T>(std::forward<U>(v));
  }

private:
  std::optional<T> val_;
  std::optional<unexpected<E>> unexpect_;
  bool has_val_;
};

template <class T1, class E1, class T2, class E2>
constexpr bool operator==(const expected<T1, E1>& x,
                          const expected<T2, E2>& y) {
  return x.has_value() != y.has_value()
             ? false
             : (!x.has_value() ? x.error() == y.error() : *x == *y);
}

template <class T1, class E1, class T2, class E2>
constexpr bool operator!=(const expected<T1, E1>& x,
                          const expected<T2, E2>& y) {
  return x.has_value() != y.has_value()
             ? true
             : (!x.has_value() ? x.error() != y.error() : *x != *y);
}

template <class T1, class E1, class T2>
constexpr bool operator==(const expected<T1, E1>& x, const T2& v) {
  return x.has_value() ? *x == v : false;
}

template <class T1, class E1, class T2>
constexpr bool operator==(const T2& v, const expected<T1, E1>& x) {
  return x.has_value() ? *x == v : false;
}

template <class T1, class E1, class T2>
constexpr bool operator!=(const expected<T1, E1>& x, const T2& v) {
  return x.has_value() ? *x != v : true;
}

template <class T1, class E1, class T2>
constexpr bool operator!=(const T2& v, const expected<T1, E1>& x) {
  return x.has_value() ? *x != v : true;
}

} // namespace exp

#endif
