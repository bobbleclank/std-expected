#ifndef INCLUDE_BC_EXPECTED_H
#define INCLUDE_BC_EXPECTED_H

#define BC_STD_EXPECTED_VERSION_MAJOR 1
#define BC_STD_EXPECTED_VERSION_MINOR 1

#include <exception>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

namespace bc {

template <class T, class E>
class expected;
template <class E>
class unexpected;

struct unexpect_t {
  explicit unexpect_t() = default;
};

inline constexpr unexpect_t unexpect{};

template <class E>
class bad_expected_access;

template <>
class bad_expected_access<void> : public std::exception {
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

namespace detail {

template <class E, class Err>
using is_constructible_from_unexpected =
    std::disjunction<std::is_constructible<E, unexpected<Err>&>,
                     std::is_constructible<E, unexpected<Err>&&>,
                     std::is_constructible<E, const unexpected<Err>&>,
                     std::is_constructible<E, const unexpected<Err>&&>,
                     std::is_convertible<unexpected<Err>&, E>,
                     std::is_convertible<unexpected<Err>&&, E>,
                     std::is_convertible<const unexpected<Err>&, E>,
                     std::is_convertible<const unexpected<Err>&&, E>>;

template <class E, class Err, class Err_qualified>
using enable_unexpected_unexpected_constructor =
    std::enable_if_t<std::is_constructible_v<E, Err_qualified> &&
                     !is_constructible_from_unexpected<E, Err>::value>;

} // namespace detail

template <class E>
class unexpected {
public:
  static_assert(!std::is_same_v<E, void>);
  static_assert(!std::is_reference_v<E>);

  unexpected() = delete;

  constexpr unexpected(const unexpected&) = default;
  constexpr unexpected(unexpected&&) = default;

  template <class Err,
            detail::enable_unexpected_unexpected_constructor<
                E, Err, const Err&>* = nullptr,
            std::enable_if_t<std::is_convertible_v<const Err&, E>>* = nullptr>
  constexpr unexpected(const unexpected<Err>& other) : val_(other.value()) {}

  template <class Err,
            detail::enable_unexpected_unexpected_constructor<
                E, Err, const Err&>* = nullptr,
            std::enable_if_t<!std::is_convertible_v<const Err&, E>>* = nullptr>
  constexpr explicit unexpected(const unexpected<Err>& other)
      : val_(other.value()) {}

  template <class Err,
            detail::enable_unexpected_unexpected_constructor<E, Err, Err&&>* =
                nullptr,
            std::enable_if_t<std::is_convertible_v<Err&&, E>>* = nullptr>
  // NOLINTNEXTLINE(*-rvalue-reference-param-not-moved): Moved via value
  constexpr unexpected(unexpected<Err>&& other)
      : val_(std::move(other.value())) {}

  template <class Err,
            detail::enable_unexpected_unexpected_constructor<E, Err, Err&&>* =
                nullptr,
            std::enable_if_t<!std::is_convertible_v<Err&&, E>>* = nullptr>
  // NOLINTNEXTLINE(*-rvalue-reference-param-not-moved): Moved via value
  constexpr explicit unexpected(unexpected<Err>&& other)
      : val_(std::move(other.value())) {}

  template <
      class Err = E,
      std::enable_if_t<
          std::is_constructible_v<E, Err&&> &&
          !std::is_same_v<std::remove_cvref_t<Err>, std::in_place_t> &&
          !std::is_same_v<std::remove_cvref_t<Err>, unexpected<E>>>* = nullptr>
  constexpr explicit unexpected(Err&& val) : val_(std::forward<Err>(val)) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr explicit unexpected(std::in_place_t, Args&&... args)
      : val_(std::forward<Args>(args)...) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                E, std::initializer_list<U>&, Args&&...>>* = nullptr>
  constexpr explicit unexpected(std::in_place_t, std::initializer_list<U> il,
                                Args&&... args)
      : val_(il, std::forward<Args>(args)...) {}

  ~unexpected() = default;

  constexpr unexpected& operator=(const unexpected&) = default;
  constexpr unexpected& operator=(unexpected&&) = default;

  template <class Err,
            std::enable_if_t<std::is_assignable_v<E&, const Err&>>* = nullptr>
  constexpr unexpected& operator=(const unexpected<Err>& other) {
    val_ = other.value();
    return *this;
  }

  template <class Err,
            std::enable_if_t<std::is_assignable_v<E&, Err&&>>* = nullptr>
  // NOLINTNEXTLINE(*-rvalue-reference-param-not-moved): Moved via value
  constexpr unexpected& operator=(unexpected<Err>&& other) {
    val_ = std::move(other.value());
    return *this;
  }

  constexpr const E& value() const& noexcept { return val_; }
  constexpr E& value() & noexcept { return val_; }
  constexpr const E&& value() const&& noexcept { return std::move(val_); }
  constexpr E&& value() && noexcept { return std::move(val_); }

  template <class E1 = E, std::enable_if_t<std::is_swappable_v<E1>>* = nullptr>
  void swap(unexpected& other) noexcept(std::is_nothrow_swappable_v<E>) {
    using std::swap;
    swap(val_, other.val_);
  }

private:
  E val_;
};

template <class E>
unexpected(E) -> unexpected<E>;

template <class E1, class E2>
constexpr bool operator==(const unexpected<E1>& x, const unexpected<E2>& y) {
  return x.value() == y.value();
}

template <class E1, class E2>
constexpr bool operator!=(const unexpected<E1>& x, const unexpected<E2>& y) {
  return x.value() != y.value();
}

template <class E, std::enable_if_t<std::is_swappable_v<E>>* = nullptr>
void swap(unexpected<E>& x, unexpected<E>& y) noexcept(noexcept(x.swap(y))) {
  x.swap(y);
}

namespace detail {

template <class T>
using is_default_constructible_or_void =
    std::disjunction<std::is_void<T>, std::is_default_constructible<T>>;

template <class T>
inline constexpr bool is_default_constructible_or_void_v =
    is_default_constructible_or_void<T>::value;

template <class T>
using is_copy_constructible_or_void =
    std::disjunction<std::is_void<T>, std::is_copy_constructible<T>>;

template <class T>
inline constexpr bool is_copy_constructible_or_void_v =
    is_copy_constructible_or_void<T>::value;

template <class T>
using is_trivially_copy_constructible_or_void =
    std::disjunction<std::is_void<T>, std::is_trivially_copy_constructible<T>>;

template <class T>
inline constexpr bool is_trivially_copy_constructible_or_void_v =
    is_trivially_copy_constructible_or_void<T>::value;

template <class T>
using is_move_constructible_or_void =
    std::disjunction<std::is_void<T>, std::is_move_constructible<T>>;

template <class T>
inline constexpr bool is_move_constructible_or_void_v =
    is_move_constructible_or_void<T>::value;

template <class T>
using is_trivially_move_constructible_or_void =
    std::disjunction<std::is_void<T>, std::is_trivially_move_constructible<T>>;

template <class T>
inline constexpr bool is_trivially_move_constructible_or_void_v =
    is_trivially_move_constructible_or_void<T>::value;

template <class T>
using is_nothrow_move_constructible_or_void =
    std::disjunction<std::is_void<T>, std::is_nothrow_move_constructible<T>>;

template <class T>
inline constexpr bool is_nothrow_move_constructible_or_void_v =
    is_nothrow_move_constructible_or_void<T>::value;

template <class T>
using is_copy_assignable_or_void =
    std::disjunction<std::is_void<T>, std::is_copy_assignable<T>>;

template <class T>
inline constexpr bool is_copy_assignable_or_void_v =
    is_copy_assignable_or_void<T>::value;

template <class T>
using is_trivially_copy_assignable_or_void =
    std::disjunction<std::is_void<T>, std::is_trivially_copy_assignable<T>>;

template <class T>
inline constexpr bool is_trivially_copy_assignable_or_void_v =
    is_trivially_copy_assignable_or_void<T>::value;

template <class T>
using is_move_assignable_or_void =
    std::disjunction<std::is_void<T>, std::is_move_assignable<T>>;

template <class T>
inline constexpr bool is_move_assignable_or_void_v =
    is_move_assignable_or_void<T>::value;

template <class T>
using is_trivially_move_assignable_or_void =
    std::disjunction<std::is_void<T>, std::is_trivially_move_assignable<T>>;

template <class T>
inline constexpr bool is_trivially_move_assignable_or_void_v =
    is_trivially_move_assignable_or_void<T>::value;

template <class T>
using is_nothrow_move_assignable_or_void =
    std::disjunction<std::is_void<T>, std::is_nothrow_move_assignable<T>>;

template <class T>
inline constexpr bool is_nothrow_move_assignable_or_void_v =
    is_nothrow_move_assignable_or_void<T>::value;

template <class T>
using is_trivially_destructible_or_void =
    std::disjunction<std::is_void<T>, std::is_trivially_destructible<T>>;

template <class T>
inline constexpr bool is_trivially_destructible_or_void_v =
    is_trivially_destructible_or_void<T>::value;

template <class T>
using is_swappable_or_void =
    std::disjunction<std::is_void<T>, std::is_swappable<T>>;

template <class T>
inline constexpr bool is_swappable_or_void_v = is_swappable_or_void<T>::value;

template <class T>
using is_nothrow_swappable_or_void =
    std::disjunction<std::is_void<T>, std::is_nothrow_swappable<T>>;

template <class T>
inline constexpr bool is_nothrow_swappable_or_void_v =
    is_nothrow_swappable_or_void<T>::value;

template <class T, class U, class G>
using is_constructible_from_expected =
    std::disjunction<std::is_constructible<T, expected<U, G>&>,
                     std::is_constructible<T, expected<U, G>&&>,
                     std::is_constructible<T, const expected<U, G>&>,
                     std::is_constructible<T, const expected<U, G>&&>,
                     std::is_convertible<expected<U, G>&, T>,
                     std::is_convertible<expected<U, G>&&, T>,
                     std::is_convertible<const expected<U, G>&, T>,
                     std::is_convertible<const expected<U, G>&&, T>>;

template <class T, class E, class U, class G, class U_qualified,
          class G_qualified>
using enable_expected_expected_constructor = std::enable_if_t<
    ((std::is_void_v<T> && std::is_void_v<U>) ||
     (std::is_constructible_v<T, U_qualified> &&
      !is_constructible_from_expected<T, U, G>::value)) &&
    std::is_constructible_v<E, G_qualified> &&
    !is_constructible_from_expected<unexpected<E>, U, G>::value>;

template <class T, class E, class U, class G, class G_qualified>
using enable_expected_expected_void_constructor = std::enable_if_t<
    std::is_void_v<T> && std::is_void_v<U> &&
    std::is_constructible_v<E, G_qualified> &&
    !is_constructible_from_expected<unexpected<E>, U, G>::value>;

template <class T, class E, class U>
using enable_expected_value_constructor =
    std::enable_if_t<!std::is_void_v<T> && std::is_constructible_v<T, U&&> &&
                     !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                     !std::is_same_v<expected<T, E>, std::remove_cvref_t<U>> &&
                     !std::is_same_v<unexpected<E>, std::remove_cvref_t<U>>>;

struct uninit_t {
  explicit uninit_t() = default;
};

inline constexpr uninit_t uninit{};

// Storage of values. Trivially destructible if both T and E are.
// clang-format off
template <class T, class E,
          bool = is_trivially_destructible_or_void_v<T> &&
                 std::is_trivially_destructible_v<E>>
struct expected_storage_base;
// clang-format on

// Either T, E or both are not trivially destructible.
template <class T, class E>
struct expected_storage_base<T, E, false> {
  constexpr expected_storage_base() : val_(), has_val_(true) {}

  expected_storage_base(const expected_storage_base&) = delete;
  expected_storage_base(expected_storage_base&&) = delete;

  constexpr explicit expected_storage_base(uninit_t)
      : uninit_(), has_val_(false) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<T, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(std::in_place_t, Args&&... args)
      : val_(std::forward<Args>(args)...), has_val_(true) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                T, std::initializer_list<U>&, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(std::in_place_t,
                                           std::initializer_list<U> il,
                                           Args&&... args)
      : val_(il, std::forward<Args>(args)...), has_val_(true) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args&&... args)
      : unexpect_(std::in_place, std::forward<Args>(args)...),
        has_val_(false) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                E, std::initializer_list<U>&, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t,
                                           std::initializer_list<U> il,
                                           Args&&... args)
      : unexpect_(std::in_place, il, std::forward<Args>(args)...),
        has_val_(false) {}

  ~expected_storage_base() {
    if (has_val_) {
      if constexpr (!std::is_trivially_destructible_v<T>)
        val_.~T();
    } else {
      if constexpr (!std::is_trivially_destructible_v<E>)
        unexpect_.~unexpected<E>();
    }
  }

  expected_storage_base& operator=(const expected_storage_base&) = delete;
  expected_storage_base& operator=(expected_storage_base&&) = delete;

  union {
    T val_;
    unexpected<E> unexpect_;
    char uninit_;
  };
  bool has_val_;
};

// Both T and E are trivially destructible.
template <class T, class E>
struct expected_storage_base<T, E, true> {
  constexpr expected_storage_base() : val_(), has_val_(true) {}

  expected_storage_base(const expected_storage_base&) = default;
  expected_storage_base(expected_storage_base&&) = default;

  constexpr explicit expected_storage_base(uninit_t)
      : uninit_(), has_val_(false) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<T, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(std::in_place_t, Args&&... args)
      : val_(std::forward<Args>(args)...), has_val_(true) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                T, std::initializer_list<U>&, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(std::in_place_t,
                                           std::initializer_list<U> il,
                                           Args&&... args)
      : val_(il, std::forward<Args>(args)...), has_val_(true) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args&&... args)
      : unexpect_(std::in_place, std::forward<Args>(args)...),
        has_val_(false) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                E, std::initializer_list<U>&, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t,
                                           std::initializer_list<U> il,
                                           Args&&... args)
      : unexpect_(std::in_place, il, std::forward<Args>(args)...),
        has_val_(false) {}

  ~expected_storage_base() = default;

  expected_storage_base& operator=(const expected_storage_base&) = default;
  expected_storage_base& operator=(expected_storage_base&&) = default;

  union {
    T val_;
    unexpected<E> unexpect_;
    char uninit_;
  };
  bool has_val_;
};

// T is void, E is not trivially destructible.
template <class E>
struct expected_storage_base<void, E, false> {
  constexpr expected_storage_base() : dummy_(), has_val_(true) {}

  expected_storage_base(const expected_storage_base&) = delete;
  expected_storage_base(expected_storage_base&&) = delete;

  constexpr explicit expected_storage_base(uninit_t)
      : uninit_(), has_val_(false) {}

  constexpr explicit expected_storage_base(std::in_place_t)
      : dummy_(), has_val_(true) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args&&... args)
      : unexpect_(std::in_place, std::forward<Args>(args)...),
        has_val_(false) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                E, std::initializer_list<U>&, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t,
                                           std::initializer_list<U> il,
                                           Args&&... args)
      : unexpect_(std::in_place, il, std::forward<Args>(args)...),
        has_val_(false) {}

  ~expected_storage_base() {
    if (!has_val_) {
      unexpect_.~unexpected<E>();
    }
  }

  expected_storage_base& operator=(const expected_storage_base&) = delete;
  expected_storage_base& operator=(expected_storage_base&&) = delete;

  struct dummy {};
  union {
    dummy dummy_;
    unexpected<E> unexpect_;
    char uninit_;
  };
  bool has_val_;
};

// T is void, E is trivially destructible.
template <class E>
struct expected_storage_base<void, E, true> {
  constexpr expected_storage_base() : dummy_(), has_val_(true) {}

  expected_storage_base(const expected_storage_base&) = default;
  expected_storage_base(expected_storage_base&&) = default;

  constexpr explicit expected_storage_base(uninit_t)
      : uninit_(), has_val_(false) {}

  constexpr explicit expected_storage_base(std::in_place_t)
      : dummy_(), has_val_(true) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args&&... args)
      : unexpect_(std::in_place, std::forward<Args>(args)...),
        has_val_(false) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                E, std::initializer_list<U>&, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t,
                                           std::initializer_list<U> il,
                                           Args&&... args)
      : unexpect_(std::in_place, il, std::forward<Args>(args)...),
        has_val_(false) {}

  ~expected_storage_base() = default;

  expected_storage_base& operator=(const expected_storage_base&) = default;
  expected_storage_base& operator=(expected_storage_base&&) = default;

  struct dummy {};
  union {
    dummy dummy_;
    unexpected<E> unexpect_;
    char uninit_;
  };
  bool has_val_;
};

// Construction and assignment.
template <class T, class E>
struct expected_operations_base : expected_storage_base<T, E> {
  using base_type = expected_storage_base<T, E>;

  using base_type::base_type;

  template <class... Args>
  constexpr void construct(std::in_place_t, Args&&... args) {
    std::construct_at(std::addressof(this->val_), std::forward<Args>(args)...);
    this->has_val_ = true;
  }

  template <class... Args>
  constexpr void construct(unexpect_t, Args&&... args) {
    std::construct_at(std::addressof(this->unexpect_),
                      std::forward<Args>(args)...);
    this->has_val_ = false;
  }

  constexpr void destroy(std::in_place_t) {
    if constexpr (!std::is_trivially_destructible_v<T>)
      this->val_.~T();
  }

  constexpr void destroy(unexpect_t) {
    if constexpr (!std::is_trivially_destructible_v<E>)
      this->unexpect_.~unexpected<E>();
  }

  template <class That>
  constexpr void construct_from(That&& other) {
    if (other.has_val_) {
      construct(std::in_place, std::forward<That>(other).val_);
    } else {
      construct(unexpect, std::forward<That>(other).unexpect_);
    }
  }

  template <class That>
  constexpr void construct_from_ex(That&& other) {
    if (other.has_value()) {
      construct(std::in_place, *std::forward<That>(other));
    } else {
      construct(unexpect, std::forward<That>(other).error());
    }
  }

  void assign(const expected_operations_base& other) {
    if (this->has_val_) {
      if (other.has_val_) {
        this->val_ = other.val_; // This can throw.
      } else {
        if constexpr (std::is_nothrow_copy_constructible_v<E>) {
          destroy(std::in_place);
          construct(unexpect, other.unexpect_);
        } else if constexpr (std::is_nothrow_move_constructible_v<E>) {
          unexpected<E> tmp = other.unexpect_; // This can throw.
          destroy(std::in_place);
          construct(unexpect, std::move(tmp));
        } else { // std::is_nothrow_move_constructible_v<T>
          T tmp = std::move(this->val_);
          destroy(std::in_place);
          try {
            construct(unexpect, other.unexpect_); // This can throw.
          } catch (...) {
            construct(std::in_place, std::move(tmp));
            throw;
          }
        }
      }
    } else {
      if (other.has_val_) {
        if constexpr (std::is_nothrow_copy_constructible_v<T>) {
          destroy(unexpect);
          construct(std::in_place, other.val_);
        } else if constexpr (std::is_nothrow_move_constructible_v<T>) {
          T tmp = other.val_; // This can throw.
          destroy(unexpect);
          construct(std::in_place, std::move(tmp));
        } else { // std::is_nothrow_move_constructible_v<E>
          unexpected<E> tmp = std::move(this->unexpect_);
          destroy(unexpect);
          try {
            construct(std::in_place, other.val_); // This can throw.
          } catch (...) {
            construct(unexpect, std::move(tmp));
            throw;
          }
        }
      } else {
        this->unexpect_ = other.unexpect_; // This can throw.
      }
    }
  }

  void assign(expected_operations_base&& other) {
    if (this->has_val_) {
      if (other.has_val_) {
        this->val_ = std::move(other).val_; // This can throw.
      } else {
        if constexpr (std::is_nothrow_move_constructible_v<E>) {
          destroy(std::in_place);
          construct(unexpect, std::move(other).unexpect_);
        } else { // std::is_nothrow_move_constructible_v<T>
          T tmp = std::move(this->val_);
          destroy(std::in_place);
          try {
            construct(unexpect, std::move(other).unexpect_); // This can throw.
          } catch (...) {
            construct(std::in_place, std::move(tmp));
            throw;
          }
        }
      }
    } else {
      if (other.has_val_) {
        if constexpr (std::is_nothrow_move_constructible_v<T>) {
          destroy(unexpect);
          construct(std::in_place, std::move(other).val_);
        } else { // std::is_nothrow_move_constructible_v<E>
          unexpected<E> tmp = std::move(this->unexpect_);
          destroy(unexpect);
          try {
            construct(std::in_place, std::move(other).val_); // This can throw.
          } catch (...) {
            construct(unexpect, std::move(tmp));
            throw;
          }
        }
      } else {
        this->unexpect_ = std::move(other).unexpect_; // This can throw.
      }
    }
  }

  void swap_impl(expected_operations_base& other) {
    if (this->has_val_) {
      if (other.has_val_) {
        using std::swap;
        swap(this->val_, other.val_);
      } else {
        if constexpr (std::is_nothrow_move_constructible_v<E>) {
          unexpected<E> tmp = std::move(other.unexpect_);
          other.destroy(unexpect);
          try {
            other.construct(std::in_place,
                            std::move(*this).val_); // This can throw.
            destroy(std::in_place);
            construct(unexpect, std::move(tmp));
          } catch (...) {
            other.construct(unexpect, std::move(tmp));
            throw;
          }
        } else { // std::is_nothrow_move_constructible_v<T>
          T tmp = std::move(this->val_);
          destroy(std::in_place);
          try {
            construct(unexpect, std::move(other).unexpect_); // This can throw.
            other.destroy(unexpect);
            other.construct(std::in_place, std::move(tmp));
          } catch (...) {
            construct(std::in_place, std::move(tmp));
            throw;
          }
        }
      }
    } else {
      if (other.has_val_) {
        other.swap_impl(*this);
      } else {
        using std::swap;
        swap(this->unexpect_, other.unexpect_);
      }
    }
  }
};

template <class E>
struct expected_operations_base<void, E> : expected_storage_base<void, E> {
  using base_type = expected_storage_base<void, E>;

  using base_type::base_type;

  template <class... Args>
  constexpr void construct(std::in_place_t) {
    this->has_val_ = true;
  }

  template <class... Args>
  constexpr void construct(unexpect_t, Args&&... args) {
    std::construct_at(std::addressof(this->unexpect_),
                      std::forward<Args>(args)...);
    this->has_val_ = false;
  }

  constexpr void destroy(std::in_place_t) {}

  constexpr void destroy(unexpect_t) {
    if constexpr (!std::is_trivially_destructible_v<E>)
      this->unexpect_.~unexpected<E>();
  }

  template <class That>
  constexpr void construct_from(That&& other) {
    if (other.has_val_) {
      construct(std::in_place);
    } else {
      construct(unexpect, std::forward<That>(other).unexpect_);
    }
  }

  template <class That>
  constexpr void construct_from_ex(That&& other) {
    if (other.has_value()) {
      construct(std::in_place);
    } else {
      construct(unexpect, std::forward<That>(other).error());
    }
  }

  void assign(const expected_operations_base& other) {
    if (this->has_val_) {
      if (other.has_val_) {
        // Nothing to do.
      } else {
        destroy(std::in_place);
        construct(unexpect, other.unexpect_); // This can throw.
      }
    } else {
      if (other.has_val_) {
        destroy(unexpect);
        construct(std::in_place);
      } else {
        this->unexpect_ = other.unexpect_; // This can throw.
      }
    }
  }

  void assign(expected_operations_base&& other) {
    if (this->has_val_) {
      if (other.has_val_) {
        // Nothing to do.
      } else {
        destroy(std::in_place);
        construct(unexpect, std::move(other).unexpect_); // This can throw.
      }
    } else {
      if (other.has_val_) {
        destroy(unexpect);
        construct(std::in_place);
      } else {
        this->unexpect_ = std::move(other).unexpect_; // This can throw.
      }
    }
  }

  void swap_impl(expected_operations_base& other) {
    if (this->has_val_) {
      if (other.has_val_) {
        // Nothing to do.
      } else {
        destroy(std::in_place);
        construct(unexpect, std::move(other).unexpect_); // This can throw.
        other.destroy(unexpect);
        other.construct(std::in_place);
      }
    } else {
      if (other.has_val_) {
        other.swap_impl(*this);
      } else {
        using std::swap;
        swap(this->unexpect_, other.unexpect_);
      }
    }
  }
};

// Copy constructor. Trivially copy constructible if both T and E are.
// clang-format off
template <class T, class E,
          bool = is_trivially_copy_constructible_or_void_v<T> &&
                 std::is_trivially_copy_constructible_v<E>>
struct expected_copy_base;
// clang-format on

// Both T and E are trivially copy constructible.
template <class T, class E>
struct expected_copy_base<T, E, true> : expected_operations_base<T, E> {
  using base_type = expected_operations_base<T, E>;

  using base_type::base_type;
};

// Either T, E or both are not trivially copy constructible.
template <class T, class E>
struct expected_copy_base<T, E, false> : expected_operations_base<T, E> {
  using base_type = expected_operations_base<T, E>;

  using base_type::base_type;

  expected_copy_base() = default;

  expected_copy_base(const expected_copy_base& other) : base_type(uninit) {
    this->construct_from(other);
  }

  expected_copy_base(expected_copy_base&&) = default;
  ~expected_copy_base() = default;
  expected_copy_base& operator=(const expected_copy_base&) = default;
  expected_copy_base& operator=(expected_copy_base&&) = default;
};

// Move constructor. Trivially move constructible if both T and E are.
// clang-format off
template <class T, class E,
          bool = is_trivially_move_constructible_or_void_v<T> &&
                 std::is_trivially_move_constructible_v<E>>
struct expected_move_base;
// clang-format on

// Both T and E are trivially move constructible.
template <class T, class E>
struct expected_move_base<T, E, true> : expected_copy_base<T, E> {
  using base_type = expected_copy_base<T, E>;

  using base_type::base_type;
};

// Either T, E or both are not trivially move constructible.
template <class T, class E>
struct expected_move_base<T, E, false> : expected_copy_base<T, E> {
  using base_type = expected_copy_base<T, E>;

  using base_type::base_type;

  expected_move_base() = default;
  expected_move_base(const expected_move_base&) = default;

  expected_move_base(expected_move_base&& other) noexcept(
      // clang-format off
      is_nothrow_move_constructible_or_void_v<T> &&
      std::is_nothrow_move_constructible_v<E>)
      // clang-format on
      : base_type(uninit) {
    this->construct_from(std::move(other));
  }

  ~expected_move_base() = default;
  expected_move_base& operator=(const expected_move_base&) = default;
  expected_move_base& operator=(expected_move_base&&) = default;
};

// Copy assignment operator. Trivially copy assignable if both T and E are.
// clang-format off
template <class T, class E,
          bool = is_trivially_copy_assignable_or_void_v<T> &&
                 is_trivially_copy_constructible_or_void_v<T> &&
                 is_trivially_destructible_or_void_v<T> &&
                 std::is_trivially_copy_assignable_v<E> &&
                 std::is_trivially_copy_constructible_v<E> &&
                 std::is_trivially_destructible_v<E>>
struct expected_copy_assign_base;
// clang-format on

// Both T and E are trivially copy assignable.
template <class T, class E>
struct expected_copy_assign_base<T, E, true> : expected_move_base<T, E> {
  using base_type = expected_move_base<T, E>;

  using base_type::base_type;
};

// Either T, E or both are not trivially copy assignable.
template <class T, class E>
struct expected_copy_assign_base<T, E, false> : expected_move_base<T, E> {
  using base_type = expected_move_base<T, E>;

  using base_type::base_type;

  expected_copy_assign_base() = default;
  expected_copy_assign_base(const expected_copy_assign_base&) = default;
  expected_copy_assign_base(expected_copy_assign_base&&) = default;
  ~expected_copy_assign_base() = default;

  expected_copy_assign_base& operator=(const expected_copy_assign_base& other) {
    this->assign(other);
    return *this;
  }

  expected_copy_assign_base& operator=(expected_copy_assign_base&&) = default;
};

// Move assignment operator. Trivially move assignable if both T and E are.
// clang-format off
template <class T, class E,
          bool = is_trivially_move_assignable_or_void_v<T> &&
                 is_trivially_move_constructible_or_void_v<T> &&
                 is_trivially_destructible_or_void_v<T> &&
                 std::is_trivially_move_assignable_v<E> &&
                 std::is_trivially_move_constructible_v<E> &&
                 std::is_trivially_destructible_v<E>>
struct expected_move_assign_base;
// clang-format on

// Both T and E are trivially move assignable.
template <class T, class E>
struct expected_move_assign_base<T, E, true> : expected_copy_assign_base<T, E> {
  using base_type = expected_copy_assign_base<T, E>;

  using base_type::base_type;
};

// Either T, E or both are not trivially move assignable.
template <class T, class E>
struct expected_move_assign_base<T, E, false>
    : expected_copy_assign_base<T, E> {
  using base_type = expected_copy_assign_base<T, E>;

  using base_type::base_type;

  expected_move_assign_base() = default;
  expected_move_assign_base(const expected_move_assign_base&) = default;
  expected_move_assign_base(expected_move_assign_base&&) = default;
  ~expected_move_assign_base() = default;
  expected_move_assign_base&
  operator=(const expected_move_assign_base&) = default;

  expected_move_assign_base&
  operator=(expected_move_assign_base&& other) noexcept(
      // clang-format off
      is_nothrow_move_assignable_or_void_v<T> &&
      is_nothrow_move_constructible_or_void_v<T> &&
      std::is_nothrow_move_assignable_v<E> &&
      std::is_nothrow_move_constructible_v<E>) {
    // clang-format on
    this->assign(std::move(other));
    return *this;
  }
};

struct construct_t {
  explicit construct_t() = default;
};

inline constexpr construct_t construct{};

// Default constructible if T is.
template <bool>
struct default_constructible_if {
  default_constructible_if() = default;

  constexpr explicit default_constructible_if(construct_t) {}
};

template <>
struct default_constructible_if<false> {
  default_constructible_if() = delete;

  constexpr explicit default_constructible_if(construct_t) {}
};

template <class T, class E>
using expected_default_constructible_if =
    default_constructible_if<is_default_constructible_or_void_v<T>>;

// Copy constructible if both T and E are.
template <bool>
struct copy_constructible_if {};

template <>
struct copy_constructible_if<false> {
  copy_constructible_if() = default;
  copy_constructible_if(const copy_constructible_if&) = delete;
  copy_constructible_if(copy_constructible_if&&) = default;
  ~copy_constructible_if() = default;
  copy_constructible_if& operator=(const copy_constructible_if&) = default;
  copy_constructible_if& operator=(copy_constructible_if&&) = default;
};

template <class T, class E>
using expected_copy_constructible_if =
    copy_constructible_if<is_copy_constructible_or_void_v<T> &&
                          std::is_copy_constructible_v<E>>;

// Move constructible if both T and E are.
template <bool>
struct move_constructible_if {};

template <>
struct move_constructible_if<false> {
  move_constructible_if() = default;
  move_constructible_if(const move_constructible_if&) = default;
  move_constructible_if(move_constructible_if&&) = delete;
  ~move_constructible_if() = default;
  move_constructible_if& operator=(const move_constructible_if&) = default;
  move_constructible_if& operator=(move_constructible_if&&) = default;
};

template <class T, class E>
using expected_move_constructible_if =
    move_constructible_if<is_move_constructible_or_void_v<T> &&
                          std::is_move_constructible_v<E>>;

// Copy assignable if both T and E are.
template <bool>
struct copy_assignable_if {};

template <>
struct copy_assignable_if<false> {
  copy_assignable_if() = default;
  copy_assignable_if(const copy_assignable_if&) = default;
  copy_assignable_if(copy_assignable_if&&) = default;
  ~copy_assignable_if() = default;
  copy_assignable_if& operator=(const copy_assignable_if&) = delete;
  copy_assignable_if& operator=(copy_assignable_if&&) = default;
};

template <class T, class E>
using expected_copy_assignable_if = copy_assignable_if<
    is_copy_constructible_or_void_v<T> && std::is_copy_constructible_v<E> &&
    is_copy_assignable_or_void_v<T> && std::is_copy_assignable_v<E> &&
    (is_nothrow_move_constructible_or_void_v<T> ||
     std::is_nothrow_move_constructible_v<E>)>;

// Move assignable if both T and E are.
template <bool>
struct move_assignable_if {};

template <>
struct move_assignable_if<false> {
  move_assignable_if() = default;
  move_assignable_if(const move_assignable_if&) = default;
  move_assignable_if(move_assignable_if&&) = default;
  ~move_assignable_if() = default;
  move_assignable_if& operator=(const move_assignable_if&) = default;
  move_assignable_if& operator=(move_assignable_if&&) = delete;
};

template <class T, class E>
using expected_move_assignable_if = move_assignable_if<
    is_move_constructible_or_void_v<T> && std::is_move_constructible_v<E> &&
    is_move_assignable_or_void_v<T> && std::is_move_assignable_v<E> &&
    (is_nothrow_move_constructible_or_void_v<T> ||
     std::is_nothrow_move_constructible_v<E>)>;

} // namespace detail

template <class T, class E>
class expected : private detail::expected_move_assign_base<T, E>,
                 private detail::expected_default_constructible_if<T, E>,
                 private detail::expected_copy_constructible_if<T, E>,
                 private detail::expected_move_constructible_if<T, E>,
                 private detail::expected_copy_assignable_if<T, E>,
                 private detail::expected_move_assignable_if<T, E> {
  using base_type = detail::expected_move_assign_base<T, E>;
  using ctor_base = detail::expected_default_constructible_if<T, E>;

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

  template <class U>
  using rebind = expected<U, error_type>;

  constexpr expected() = default;

  constexpr expected(const expected&) = default;
  constexpr expected(expected&&) = default;

  template <class U, class G,
            detail::enable_expected_expected_void_constructor<
                T, E, U, G, const G&>* = nullptr,
            std::enable_if_t<std::is_convertible_v<const G&, E>>* = nullptr>
  constexpr expected(const expected<U, G>& other)
      : base_type(detail::uninit), ctor_base(detail::construct) {
    this->construct_from_ex(other);
  }

  template <class U, class G,
            detail::enable_expected_expected_void_constructor<
                T, E, U, G, const G&>* = nullptr,
            std::enable_if_t<!std::is_convertible_v<const G&, E>>* = nullptr>
  constexpr explicit expected(const expected<U, G>& other)
      : base_type(detail::uninit), ctor_base(detail::construct) {
    this->construct_from_ex(other);
  }

  template <class U, class G,
            detail::enable_expected_expected_constructor<T, E, U, G, const U&,
                                                         const G&>* = nullptr,
            // clang-format off
            std::enable_if_t<(std::is_void_v<T> || std::is_void_v<U> ||
                              std::is_convertible_v<const U&, T>) &&
                             std::is_convertible_v<const G&, E>>* = nullptr>
  // clang-format on
  constexpr expected(const expected<U, G>& other)
      : base_type(detail::uninit), ctor_base(detail::construct) {
    this->construct_from_ex(other);
  }

  template <class U, class G,
            detail::enable_expected_expected_constructor<T, E, U, G, const U&,
                                                         const G&>* = nullptr,
            std::enable_if_t<(!std::is_void_v<T> && !std::is_void_v<U> &&
                              !std::is_convertible_v<const U&, T>) ||
                             !std::is_convertible_v<const G&, E>>* = nullptr>
  constexpr explicit expected(const expected<U, G>& other)
      : base_type(detail::uninit), ctor_base(detail::construct) {
    this->construct_from_ex(other);
  }

  template <class U, class G,
            detail::enable_expected_expected_void_constructor<T, E, U, G,
                                                              G&&>* = nullptr,
            std::enable_if_t<std::is_convertible_v<G&&, E>>* = nullptr>
  constexpr expected(expected<U, G>&& other)
      : base_type(detail::uninit), ctor_base(detail::construct) {
    this->construct_from_ex(std::move(other));
  }

  template <class U, class G,
            detail::enable_expected_expected_void_constructor<T, E, U, G,
                                                              G&&>* = nullptr,
            std::enable_if_t<!std::is_convertible_v<G&&, E>>* = nullptr>
  constexpr explicit expected(expected<U, G>&& other)
      : base_type(detail::uninit), ctor_base(detail::construct) {
    this->construct_from_ex(std::move(other));
  }

  template <class U, class G,
            detail::enable_expected_expected_constructor<T, E, U, G, U&&,
                                                         G&&>* = nullptr,
            // clang-format off
            std::enable_if_t<(std::is_void_v<T> || std::is_void_v<U> ||
                              std::is_convertible_v<U&&, T>) &&
                             std::is_convertible_v<G&&, E>>* = nullptr>
  // clang-format on
  constexpr expected(expected<U, G>&& other)
      : base_type(detail::uninit), ctor_base(detail::construct) {
    this->construct_from_ex(std::move(other));
  }

  template <class U, class G,
            detail::enable_expected_expected_constructor<T, E, U, G, U&&,
                                                         G&&>* = nullptr,
            std::enable_if_t<(!std::is_void_v<T> && !std::is_void_v<U> &&
                              !std::is_convertible_v<U&&, T>) ||
                             !std::is_convertible_v<G&&, E>>* = nullptr>
  constexpr explicit expected(expected<U, G>&& other)
      : base_type(detail::uninit), ctor_base(detail::construct) {
    this->construct_from_ex(std::move(other));
  }

  template <class U = T,
            detail::enable_expected_value_constructor<T, E, U>* = nullptr,
            std::enable_if_t<std::is_convertible_v<U&&, T>>* = nullptr>
  constexpr expected(U&& v)
      : base_type(std::in_place, std::forward<U>(v)),
        ctor_base(detail::construct) {}

  template <class U = T,
            detail::enable_expected_value_constructor<T, E, U>* = nullptr,
            std::enable_if_t<!std::is_convertible_v<U&&, T>>* = nullptr>
  constexpr explicit expected(U&& v)
      : base_type(std::in_place, std::forward<U>(v)),
        ctor_base(detail::construct) {}

  template <class G = E,
            std::enable_if_t<std::is_constructible_v<E, const G&>>* = nullptr,
            std::enable_if_t<std::is_convertible_v<const G&, E>>* = nullptr>
  constexpr expected(const unexpected<G>& e)
      : base_type(unexpect, e.value()), ctor_base(detail::construct) {}

  template <class G = E,
            std::enable_if_t<std::is_constructible_v<E, const G&>>* = nullptr,
            std::enable_if_t<!std::is_convertible_v<const G&, E>>* = nullptr>
  constexpr explicit expected(const unexpected<G>& e)
      : base_type(unexpect, e.value()), ctor_base(detail::construct) {}

  template <class G = E,
            std::enable_if_t<std::is_constructible_v<E, G&&>>* = nullptr,
            std::enable_if_t<std::is_convertible_v<G&&, E>>* = nullptr>
  // NOLINTNEXTLINE(*-rvalue-reference-param-not-moved): Moved via value
  constexpr expected(unexpected<G>&& e) noexcept(
      std::is_nothrow_constructible_v<E, G&&>)
      : base_type(unexpect, std::move(e.value())),
        ctor_base(detail::construct) {}

  template <class G = E,
            std::enable_if_t<std::is_constructible_v<E, G&&>>* = nullptr,
            std::enable_if_t<!std::is_convertible_v<G&&, E>>* = nullptr>
  // NOLINTNEXTLINE(*-rvalue-reference-param-not-moved): Moved via value
  constexpr explicit expected(unexpected<G>&& e) noexcept(
      std::is_nothrow_constructible_v<E, G&&>)
      : base_type(unexpect, std::move(e.value())),
        ctor_base(detail::construct) {}

  template <
      class... Args,
      std::enable_if_t<(std::is_void_v<T> && sizeof...(Args) == 0) ||
                       (!std::is_void_v<T> &&
                        std::is_constructible_v<T, Args&&...>)>* = nullptr>
  constexpr explicit expected(std::in_place_t, Args&&... args)
      : base_type(std::in_place, std::forward<Args>(args)...),
        ctor_base(detail::construct) {}

  template <
      class U, class... Args,
      std::enable_if_t<!std::is_void_v<T> &&
                       std::is_constructible_v<T, std::initializer_list<U>&,
                                               Args&&...>>* = nullptr>
  constexpr explicit expected(std::in_place_t, std::initializer_list<U> il,
                              Args&&... args)
      : base_type(std::in_place, il, std::forward<Args>(args)...),
        ctor_base(detail::construct) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr explicit expected(unexpect_t, Args&&... args)
      : base_type(unexpect, std::forward<Args>(args)...),
        ctor_base(detail::construct) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                E, std::initializer_list<U>&, Args&&...>>* = nullptr>
  constexpr explicit expected(unexpect_t, std::initializer_list<U> il,
                              Args&&... args)
      : base_type(unexpect, il, std::forward<Args>(args)...),
        ctor_base(detail::construct) {}

  ~expected() = default;

  expected& operator=(const expected&) = default;
  expected& operator=(expected&&) = default;

  template <
      class U = T, class T1 = T,
      std::enable_if_t<!std::is_void_v<T1>>* = nullptr,
      std::enable_if_t<
          !std::is_same_v<expected<T, E>, std::remove_cvref_t<U>> &&
          !std::conjunction_v<std::is_scalar<T>,
                              std::is_same<T, std::remove_cvref_t<U>>> &&
          std::is_constructible_v<T, U&&> && std::is_assignable_v<T1&, U&&> &&
          std::is_nothrow_move_constructible_v<E>>* = nullptr>
  expected& operator=(U&& v) {
    if (this->has_val_) {
      this->val_ = std::forward<U>(v); // This can throw.
    } else {
      if constexpr (std::is_nothrow_constructible_v<T, U&&>) {
        this->destroy(unexpect);
        this->construct(std::in_place, std::forward<U>(v));
      } else { // std::is_nothrow_move_constructible_v<E>
        unexpected<E> tmp = std::move(this->unexpect_);
        this->destroy(unexpect);
        try {
          this->construct(std::in_place, std::forward<U>(v)); // This can throw.
        } catch (...) {
          this->construct(unexpect, std::move(tmp));
          throw;
        }
      }
    }
    return *this;
  }

  template <class G = E,
            std::enable_if_t<std::is_nothrow_constructible_v<E, const G&> &&
                             std::is_assignable_v<E&, const G&>>* = nullptr>
  expected& operator=(const unexpected<G>& e) {
    if (this->has_val_) {
      this->destroy(std::in_place);
      this->construct(unexpect, e.value());
    } else {
      this->unexpect_ = e; // This can throw.
    }
    return *this;
  }

  template <class G = E,
            std::enable_if_t<std::is_nothrow_constructible_v<E, G&&> &&
                             std::is_assignable_v<E&, G&&>>* = nullptr>
  expected& operator=(unexpected<G>&& e) {
    if (this->has_val_) {
      this->destroy(std::in_place);
      this->construct(unexpect, std::move(e.value()));
    } else {
      this->unexpect_ = std::move(e); // This can throw.
    }
    return *this;
  }

  template <class T1 = T, std::enable_if_t<std::is_void_v<T1>>* = nullptr>
  void emplace() {
    if (!this->has_val_) {
      this->destroy(unexpect);
      this->construct(std::in_place);
    }
  }

  template <
      class... Args, class T1 = T,
      std::enable_if_t<!std::is_void_v<T1>>* = nullptr,
      std::enable_if_t<std::is_constructible_v<T, Args&&...> &&
                       std::is_move_assignable_v<T> &&
                       (std::is_nothrow_move_constructible_v<T> ||
                        std::is_nothrow_move_constructible_v<E>)>* = nullptr>
  T1& emplace(Args&&... args) {
    if (this->has_val_) {
      this->val_ = T(std::forward<Args>(args)...); // This can throw.
    } else if constexpr (std::is_nothrow_constructible_v<T, Args&&...>) {
      this->destroy(unexpect);
      this->construct(std::in_place, std::forward<Args>(args)...);
    } else if constexpr (std::is_nothrow_move_constructible_v<T>) {
      T tmp(std::forward<Args>(args)...); // This can throw.
      this->destroy(unexpect);
      this->construct(std::in_place, std::move(tmp));
    } else { // std::is_nothrow_move_constructible_v<E>
      unexpected<E> tmp = std::move(this->unexpect_);
      this->destroy(unexpect);
      try {
        this->construct(std::in_place,
                        std::forward<Args>(args)...); // This can throw.
      } catch (...) {
        this->construct(unexpect, std::move(tmp));
        throw;
      }
    }
    return this->val_;
  }

  template <
      class U, class... Args, class T1 = T,
      std::enable_if_t<!std::is_void_v<T1>>* = nullptr,
      std::enable_if_t<
          std::is_constructible_v<T, std::initializer_list<U>&, Args&&...> &&
          std::is_move_assignable_v<T> &&
          (std::is_nothrow_move_constructible_v<T> ||
           std::is_nothrow_move_constructible_v<E>)>* = nullptr>
  T1& emplace(std::initializer_list<U> il, Args&&... args) {
    if (this->has_val_) {
      this->val_ = T(il, std::forward<Args>(args)...); // This can throw.
    } else if constexpr (std::is_nothrow_constructible_v<
                             T, std::initializer_list<U>&, Args&&...>) {
      this->destroy(unexpect);
      this->construct(std::in_place, il, std::forward<Args>(args)...);
    } else if constexpr (std::is_nothrow_move_constructible_v<T>) {
      T tmp(il, std::forward<Args>(args)...); // This can throw.
      this->destroy(unexpect);
      this->construct(std::in_place, std::move(tmp));
    } else { // std::is_nothrow_move_constructible_v<E>
      unexpected<E> tmp = std::move(this->unexpect_);
      this->destroy(unexpect);
      try {
        this->construct(std::in_place, il,
                        std::forward<Args>(args)...); // This can throw.
      } catch (...) {
        this->construct(unexpect, std::move(tmp));
        throw;
      }
    }
    return this->val_;
  }

  template <class T1 = T, std::enable_if_t<!std::is_void_v<T1>>* = nullptr>
  constexpr const T* operator->() const {
    return std::addressof(this->val_);
  }

  template <class T1 = T, std::enable_if_t<!std::is_void_v<T1>>* = nullptr>
  constexpr T* operator->() {
    return std::addressof(this->val_);
  }

  template <class T1 = T, std::enable_if_t<!std::is_void_v<T1>>* = nullptr>
  constexpr const T1& operator*() const& {
    return this->val_;
  }

  template <class T1 = T, std::enable_if_t<!std::is_void_v<T1>>* = nullptr>
  constexpr T1& operator*() & {
    return this->val_;
  }

  template <class T1 = T, std::enable_if_t<!std::is_void_v<T1>>* = nullptr>
  constexpr const T1&& operator*() const&& {
    return std::move(this->val_);
  }

  template <class T1 = T, std::enable_if_t<!std::is_void_v<T1>>* = nullptr>
  constexpr T1&& operator*() && {
    return std::move(this->val_);
  }

  constexpr explicit operator bool() const noexcept { return this->has_val_; }
  constexpr bool has_value() const noexcept { return this->has_val_; }

  template <class T1 = T, std::enable_if_t<std::is_void_v<T1>>* = nullptr>
  constexpr void value() const {
    if (!this->has_val_)
      throw bad_expected_access(this->unexpect_.value());
  }

  template <class T1 = T, std::enable_if_t<!std::is_void_v<T1>>* = nullptr>
  constexpr const T1& value() const& {
    if (!this->has_val_)
      throw bad_expected_access(this->unexpect_.value());
    return this->val_;
  }

  template <class T1 = T, std::enable_if_t<!std::is_void_v<T1>>* = nullptr>
  constexpr T1& value() & {
    if (!this->has_val_)
      throw bad_expected_access(this->unexpect_.value());
    return this->val_;
  }

  template <class T1 = T, std::enable_if_t<!std::is_void_v<T1>>* = nullptr>
  constexpr const T1&& value() const&& {
    if (!this->has_val_)
      throw bad_expected_access(std::move(this->unexpect_.value()));
    return std::move(this->val_);
  }

  template <class T1 = T, std::enable_if_t<!std::is_void_v<T1>>* = nullptr>
  constexpr T1&& value() && {
    if (!this->has_val_)
      throw bad_expected_access(std::move(this->unexpect_.value()));
    return std::move(this->val_);
  }

  constexpr const E& error() const& { return this->unexpect_.value(); }
  constexpr E& error() & { return this->unexpect_.value(); }
  constexpr const E&& error() const&& {
    return std::move(this->unexpect_.value());
  }
  constexpr E&& error() && { return std::move(this->unexpect_.value()); }

  template <class U, std::enable_if_t<std::is_copy_constructible_v<T> &&
                                      std::is_convertible_v<U&&, T>>* = nullptr>
  constexpr T value_or(U&& v) const& {
    return this->has_val_ ? this->val_ : static_cast<T>(std::forward<U>(v));
  }

  template <class U, std::enable_if_t<std::is_move_constructible_v<T> &&
                                      std::is_convertible_v<U&&, T>>* = nullptr>
  constexpr T value_or(U&& v) && {
    return this->has_val_ ? std::move(this->val_)
                          : static_cast<T>(std::forward<U>(v));
  }

  template <class T1 = T, class E1 = E,
            std::enable_if_t<
                detail::is_move_constructible_or_void_v<T1> &&
                std::is_move_constructible_v<E1> &&
                detail::is_swappable_or_void_v<T1> && std::is_swappable_v<E1> &&
                (detail::is_nothrow_move_constructible_or_void_v<T1> ||
                 std::is_nothrow_move_constructible_v<E1>)>* = nullptr>
  void swap(expected& other) noexcept(
      // clang-format off
      detail::is_nothrow_move_constructible_or_void_v<T> &&
      std::is_nothrow_move_constructible_v<E> &&
      detail::is_nothrow_swappable_or_void_v<T> &&
      std::is_nothrow_swappable_v<E>) {
    // clang-format on
    this->swap_impl(other);
  }
};

template <
    class T1, class E1, class T2, class E2,
    std::enable_if_t<!std::is_void_v<T1> && !std::is_void_v<T2>>* = nullptr>
constexpr bool operator==(const expected<T1, E1>& x,
                          const expected<T2, E2>& y) {
  return x.has_value() != y.has_value()
             ? false
             : (!x.has_value() ? x.error() == y.error() : *x == *y);
}

template <
    class T1, class E1, class T2, class E2,
    std::enable_if_t<!std::is_void_v<T1> && !std::is_void_v<T2>>* = nullptr>
constexpr bool operator!=(const expected<T1, E1>& x,
                          const expected<T2, E2>& y) {
  return x.has_value() != y.has_value()
             ? true
             : (!x.has_value() ? x.error() != y.error() : *x != *y);
}

template <class E1, class E2>
constexpr bool operator==(const expected<void, E1>& x,
                          const expected<void, E2>& y) {
  return x.has_value() != y.has_value()
             ? false
             : (!x.has_value() ? x.error() == y.error() : true);
}

template <class E1, class E2>
constexpr bool operator!=(const expected<void, E1>& x,
                          const expected<void, E2>& y) {
  return x.has_value() != y.has_value()
             ? true
             : (!x.has_value() ? x.error() != y.error() : false);
}

template <
    class T1, class E1, class T2,
    std::enable_if_t<!std::is_void_v<T1> && !std::is_void_v<T2>>* = nullptr>
constexpr bool operator==(const expected<T1, E1>& x, const T2& v) {
  return x.has_value() ? *x == v : false;
}

template <
    class T1, class E1, class T2,
    std::enable_if_t<!std::is_void_v<T1> && !std::is_void_v<T2>>* = nullptr>
constexpr bool operator==(const T2& v, const expected<T1, E1>& x) {
  return x.has_value() ? *x == v : false;
}

template <
    class T1, class E1, class T2,
    std::enable_if_t<!std::is_void_v<T1> && !std::is_void_v<T2>>* = nullptr>
constexpr bool operator!=(const expected<T1, E1>& x, const T2& v) {
  return x.has_value() ? *x != v : true;
}

template <
    class T1, class E1, class T2,
    std::enable_if_t<!std::is_void_v<T1> && !std::is_void_v<T2>>* = nullptr>
constexpr bool operator!=(const T2& v, const expected<T1, E1>& x) {
  return x.has_value() ? *x != v : true;
}

template <class T1, class E1, class E2>
constexpr bool operator==(const expected<T1, E1>& x, const unexpected<E2>& e) {
  return x.has_value() ? false : x.error() == e.value();
}

template <class T1, class E1, class E2>
constexpr bool operator==(const unexpected<E2>& e, const expected<T1, E1>& x) {
  return x.has_value() ? false : x.error() == e.value();
}

template <class T1, class E1, class E2>
constexpr bool operator!=(const expected<T1, E1>& x, const unexpected<E2>& e) {
  return x.has_value() ? true : x.error() != e.value();
}

template <class T1, class E1, class E2>
constexpr bool operator!=(const unexpected<E2>& e, const expected<T1, E1>& x) {
  return x.has_value() ? true : x.error() != e.value();
}

template <class T, class E,
          std::enable_if_t<
              detail::is_move_constructible_or_void_v<T> &&
              std::is_move_constructible_v<E> &&
              detail::is_swappable_or_void_v<T> && std::is_swappable_v<E> &&
              (detail::is_nothrow_move_constructible_or_void_v<T> ||
               std::is_nothrow_move_constructible_v<E>)>* = nullptr>
void swap(expected<T, E>& x, expected<T, E>& y) noexcept(noexcept(x.swap(y))) {
  x.swap(y);
}

} // namespace bc

#endif
