#ifndef INCLUDE_BC_EXP_EXPECTED_H
#define INCLUDE_BC_EXP_EXPECTED_H

#define BC_STD_EXPECTED_VERSION_MAJOR 0
#define BC_STD_EXPECTED_VERSION_MINOR 1

#include <exception>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

namespace bc::exp {

namespace cpp {

template <class T> struct remove_cvref {
  typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};

template <class T> using remove_cvref_t = typename remove_cvref<T>::type;

} // namespace cpp

template <class T, class E> class expected;
template <class E> class unexpected;

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

namespace internal {

template <class E, class Err, class Err_qualified>
using enable_unexpected_unexpected_constructor =
    std::enable_if_t<std::is_constructible_v<E, Err_qualified> &&

                     !std::is_constructible_v<E, unexpected<Err>&> &&
                     !std::is_constructible_v<E, unexpected<Err>&&> &&
                     !std::is_constructible_v<E, const unexpected<Err>&> &&
                     !std::is_constructible_v<E, const unexpected<Err>&&> &&
                     !std::is_convertible_v<unexpected<Err>&, E> &&
                     !std::is_convertible_v<unexpected<Err>&&, E> &&
                     !std::is_convertible_v<const unexpected<Err>&, E> &&
                     !std::is_convertible_v<const unexpected<Err>&&, E>>;

} // namespace internal

template <class E> class unexpected {
public:
  static_assert(!std::is_same_v<E, void>);
  static_assert(!std::is_reference_v<E>);

  unexpected() = delete;

  constexpr unexpected(const unexpected&) = default;
  constexpr unexpected(unexpected&&) = default;

  template <class Err,
            std::enable_if_t<std::is_convertible_v<const Err&, E>>* = nullptr,
            internal::enable_unexpected_unexpected_constructor<
                E, Err, const Err&>* = nullptr>
  constexpr unexpected(const unexpected<Err>& other) : val_(other.value()) {}

  template <class Err,
            std::enable_if_t<!std::is_convertible_v<const Err&, E>>* = nullptr,
            internal::enable_unexpected_unexpected_constructor<
                E, Err, const Err&>* = nullptr>
  constexpr explicit unexpected(const unexpected<Err>& other)
      : val_(other.value()) {}

  template <class Err,
            std::enable_if_t<std::is_convertible_v<Err&&, E>>* = nullptr,
            internal::enable_unexpected_unexpected_constructor<E, Err, Err&&>* =
                nullptr>
  constexpr unexpected(unexpected<Err>&& other)
      : val_(std::move(other.value())) {}

  template <class Err,
            std::enable_if_t<!std::is_convertible_v<Err&&, E>>* = nullptr,
            internal::enable_unexpected_unexpected_constructor<E, Err, Err&&>* =
                nullptr>
  constexpr explicit unexpected(unexpected<Err>&& other)
      : val_(std::move(other.value())) {}

  template <
      class Err = E,
      std::enable_if_t<
          std::is_constructible_v<E, Err&&> &&
          !std::is_same_v<cpp::remove_cvref_t<Err>, std::in_place_t> &&
          !std::is_same_v<cpp::remove_cvref_t<Err>, unexpected<E>>>* = nullptr>
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

  template <class Err,
            std::enable_if_t<std::is_assignable_v<E&, const Err&>>* = nullptr>
  constexpr unexpected& operator=(const unexpected<Err>& other) {
    val_ = other.value();
    return *this;
  }

  template <class Err,
            std::enable_if_t<std::is_assignable_v<E&, Err&&>>* = nullptr>
  constexpr unexpected& operator=(unexpected<Err>&& other) {
    val_ = std::move(other.value());
    return *this;
  }

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

namespace internal {

template <class T>
using is_trivially_copy_constructible_or_void =
    std::disjunction<std::is_void<T>, std::is_trivially_copy_constructible<T>>;

template <class T>
inline constexpr bool is_trivially_copy_constructible_or_void_v =
    is_trivially_copy_constructible_or_void<T>::value;

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
using is_trivially_copy_assignable_or_void =
    std::disjunction<std::is_void<T>, std::is_trivially_copy_assignable<T>>;

template <class T>
inline constexpr bool is_trivially_copy_assignable_or_void_v =
    is_trivially_copy_assignable_or_void<T>::value;

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

template <class T, class E, class U, class G, class U_qualified,
          class G_qualified>
using enable_expected_expected_constructor = std::enable_if_t<
    std::is_constructible_v<T, U_qualified> &&

    !std::is_constructible_v<T, expected<U, G>&> &&
    !std::is_constructible_v<T, expected<U, G>&&> &&
    !std::is_constructible_v<T, const expected<U, G>&> &&
    !std::is_constructible_v<T, const expected<U, G>&&> &&
    !std::is_convertible_v<expected<U, G>&, T> &&
    !std::is_convertible_v<expected<U, G>&&, T> &&
    !std::is_convertible_v<const expected<U, G>&, T> &&
    !std::is_convertible_v<const expected<U, G>&&, T> &&

    std::is_constructible_v<E, G_qualified> &&

    !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
    !std::is_constructible_v<unexpected<E>, expected<U, G>&&> &&
    !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
    !std::is_constructible_v<unexpected<E>, const expected<U, G>&&> &&
    !std::is_convertible_v<expected<U, G>&, unexpected<E>> &&
    !std::is_convertible_v<expected<U, G>&&, unexpected<E>> &&
    !std::is_convertible_v<const expected<U, G>&, unexpected<E>> &&
    !std::is_convertible_v<const expected<U, G>&&, unexpected<E>>>;

template <class T, class E, class U>
using enable_expected_value_constructor =
    std::enable_if_t<std::is_constructible_v<T, U&&> &&
                     !std::is_same_v<cpp::remove_cvref_t<U>, std::in_place_t> &&
                     !std::is_same_v<expected<T, E>, cpp::remove_cvref_t<U>> &&
                     !std::is_same_v<unexpected<E>, cpp::remove_cvref_t<U>>>;

struct uninit_t {};

inline constexpr uninit_t uninit{};

// Storage of values. Trivially destructible if both T and E are.
// clang-format off
template <class T, class E,
          bool = is_trivially_destructible_or_void_v<T> &&
                 std::is_trivially_destructible_v<E>>
struct expected_storage_base;
// clang-format on

// Either T, E or both are not trivially destructible.
template <class T, class E> struct expected_storage_base<T, E, false> {
  constexpr expected_storage_base() : val_(), has_val_(true) {}

  constexpr explicit expected_storage_base(uninit_t)
      : uninit_(), has_val_(false) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<T, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(std::in_place_t, Args&&... args)
      : val_(std::forward<Args>(args)...), has_val_(true) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                T, std::initializer_list<U>, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(std::in_place_t,
                                           std::initializer_list<U> il,
                                           Args&&... args)
      : val_(il, std::forward<Args>(args)...), has_val_(true) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args&&... args)
      : unexpect_(std::in_place, std::forward<Args>(args)...), has_val_(false) {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                E, std::initializer_list<U>, Args&&...>>* = nullptr>
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

  union {
    T val_;
    unexpected<E> unexpect_;
    char uninit_;
  };
  bool has_val_;
};

// Both T and E are trivially destructible.
template <class T, class E> struct expected_storage_base<T, E, true> {
  constexpr expected_storage_base() : val_(), has_val_(true) {}

  constexpr explicit expected_storage_base(uninit_t)
      : uninit_(), has_val_(false) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<T, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(std::in_place_t, Args&&... args)
      : val_(std::forward<Args>(args)...), has_val_(true) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                T, std::initializer_list<U>, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(std::in_place_t,
                                           std::initializer_list<U> il,
                                           Args&&... args)
      : val_(il, std::forward<Args>(args)...), has_val_(true) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args&&... args)
      : unexpect_(std::in_place, std::forward<Args>(args)...), has_val_(false) {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                E, std::initializer_list<U>, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t,
                                           std::initializer_list<U> il,
                                           Args&&... args)
      : unexpect_(std::in_place, il, std::forward<Args>(args)...),
        has_val_(false) {}

  ~expected_storage_base() = default;

  union {
    T val_;
    unexpected<E> unexpect_;
    char uninit_;
  };
  bool has_val_;
};

// T is void, E is not trivially destructible.
template <class E> struct expected_storage_base<void, E, false> {
  constexpr expected_storage_base() : dummy_(), has_val_(true) {}

  constexpr explicit expected_storage_base(uninit_t)
      : uninit_(), has_val_(false) {}

  constexpr explicit expected_storage_base(std::in_place_t)
      : dummy_(), has_val_(true) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args&&... args)
      : unexpect_(std::in_place, std::forward<Args>(args)...), has_val_(false) {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                E, std::initializer_list<U>, Args&&...>>* = nullptr>
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

  struct dummy {};
  union {
    dummy dummy_;
    unexpected<E> unexpect_;
    char uninit_;
  };
  bool has_val_;
};

// T is void, E is trivially destructible.
template <class E> struct expected_storage_base<void, E, true> {
  constexpr expected_storage_base() : dummy_(), has_val_(true) {}

  constexpr explicit expected_storage_base(uninit_t)
      : uninit_(), has_val_(false) {}

  constexpr explicit expected_storage_base(std::in_place_t)
      : dummy_(), has_val_(true) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args&&... args)
      : unexpect_(std::in_place, std::forward<Args>(args)...), has_val_(false) {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                E, std::initializer_list<U>, Args&&...>>* = nullptr>
  constexpr explicit expected_storage_base(unexpect_t,
                                           std::initializer_list<U> il,
                                           Args&&... args)
      : unexpect_(std::in_place, il, std::forward<Args>(args)...),
        has_val_(false) {}

  ~expected_storage_base() = default;

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
    ::new (std::addressof(this->val_)) T(std::forward<Args>(args)...);
    this->has_val_ = true;
  }

  template <class... Args>
  constexpr void construct(unexpect_t, Args&&... args) {
    ::new (std::addressof(this->unexpect_))
        unexpected<E>(std::forward<Args>(args)...);
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

  template <class That> void construct_from(That&& other) {
    if (other.has_val_) {
      construct(std::in_place, std::forward<That>(other).val_);
    } else {
      construct(unexpect, std::forward<That>(other).unexpect_);
    }
  }

  template <class That> void construct_from_ex(That&& other) {
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
};

template <class E>
struct expected_operations_base<void, E> : expected_storage_base<void, E> {
  using base_type = expected_storage_base<void, E>;

  using base_type::base_type;

  template <class... Args> constexpr void construct(std::in_place_t) {
    this->has_val_ = true;
  }

  template <class... Args>
  constexpr void construct(unexpect_t, Args&&... args) {
    ::new (std::addressof(this->unexpect_))
        unexpected<E>(std::forward<Args>(args)...);
    this->has_val_ = false;
  }

  constexpr void destroy(std::in_place_t) {}

  constexpr void destroy(unexpect_t) {
    if constexpr (!std::is_trivially_destructible_v<E>)
      this->unexpect_.~unexpected<E>();
  }

  template <class That> void construct_from(That&& other) {
    if (other.has_val_) {
      construct(std::in_place);
    } else {
      construct(unexpect, std::forward<That>(other).unexpect_);
    }
  }

  template <class That> void construct_from_ex(That&& other) {
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

} // namespace internal

template <class T, class E>
class expected : private internal::expected_move_assign_base<T, E> {
  using base_type = internal::expected_move_assign_base<T, E>;

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

  constexpr expected() = default;

  constexpr expected(const expected&) = default;
  constexpr expected(expected&&) = default;

  template <class U, class G,
            std::enable_if_t<std::is_convertible_v<const U&, T> &&
                             std::is_convertible_v<const G&, E>>* = nullptr,
            internal::enable_expected_expected_constructor<T, E, U, G, const U&,
                                                           const G&>* = nullptr>
  constexpr expected(const expected<U, G>& other)
      : base_type(internal::uninit) {
    this->construct_from_ex(other);
  }

  template <class U, class G,
            std::enable_if_t<!std::is_convertible_v<const U&, T> ||
                             !std::is_convertible_v<const G&, E>>* = nullptr,
            internal::enable_expected_expected_constructor<T, E, U, G, const U&,
                                                           const G&>* = nullptr>
  constexpr explicit expected(const expected<U, G>& other)
      : base_type(internal::uninit) {
    this->construct_from_ex(other);
  }

  template <class U, class G,
            std::enable_if_t<std::is_convertible_v<U&&, T> &&
                             std::is_convertible_v<G&&, E>>* = nullptr,
            internal::enable_expected_expected_constructor<T, E, U, G, U&&,
                                                           G&&>* = nullptr>
  constexpr expected(expected<U, G>&& other) : base_type(internal::uninit) {
    this->construct_from_ex(std::move(other));
  }

  template <class U, class G,
            std::enable_if_t<!std::is_convertible_v<U&&, T> ||
                             !std::is_convertible_v<G&&, E>>* = nullptr,
            internal::enable_expected_expected_constructor<T, E, U, G, U&&,
                                                           G&&>* = nullptr>
  constexpr explicit expected(expected<U, G>&& other)
      : base_type(internal::uninit) {
    this->construct_from_ex(std::move(other));
  }

  template <class U = T,
            std::enable_if_t<std::is_convertible_v<U&&, T>>* = nullptr,
            internal::enable_expected_value_constructor<T, E, U>* = nullptr>
  constexpr expected(U&& v) : base_type(std::in_place, std::forward<U>(v)) {}

  template <class U = T,
            std::enable_if_t<!std::is_convertible_v<U&&, T>>* = nullptr,
            internal::enable_expected_value_constructor<T, E, U>* = nullptr>
  constexpr explicit expected(U&& v)
      : base_type(std::in_place, std::forward<U>(v)) {}

  template <class G = E,
            std::enable_if_t<std::is_convertible_v<const G&, E>>* = nullptr,
            std::enable_if_t<std::is_constructible_v<E, const G&>>* = nullptr>
  constexpr expected(const unexpected<G>& e) : base_type(unexpect, e.value()) {}

  template <class G = E,
            std::enable_if_t<!std::is_convertible_v<const G&, E>>* = nullptr,
            std::enable_if_t<std::is_constructible_v<E, const G&>>* = nullptr>
  constexpr explicit expected(const unexpected<G>& e)
      : base_type(unexpect, e.value()) {}

  template <class G = E,
            std::enable_if_t<std::is_convertible_v<G&&, E>>* = nullptr,
            std::enable_if_t<std::is_constructible_v<E, G&&>>* = nullptr>
  constexpr expected(unexpected<G>&& e) noexcept(
      std::is_nothrow_constructible_v<E, G&&>)
      : base_type(unexpect, std::move(e.value())) {}

  template <class G = E,
            std::enable_if_t<!std::is_convertible_v<G&&, E>>* = nullptr,
            std::enable_if_t<std::is_constructible_v<E, G&&>>* = nullptr>
  constexpr explicit expected(unexpected<G>&& e) noexcept(
      std::is_nothrow_constructible_v<E, G&&>)
      : base_type(unexpect, std::move(e.value())) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<T, Args&&...>>* = nullptr>
  constexpr explicit expected(std::in_place_t, Args&&... args)
      : base_type(std::in_place, std::forward<Args>(args)...) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                T, std::initializer_list<U>, Args&&...>>* = nullptr>
  constexpr explicit expected(std::in_place_t, std::initializer_list<U> il,
                              Args&&... args)
      : base_type(std::in_place, il, std::forward<Args>(args)...) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr explicit expected(unexpect_t, Args&&... args)
      : base_type(unexpect, std::forward<Args>(args)...) {}

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                E, std::initializer_list<U>, Args&&...>>* = nullptr>
  constexpr explicit expected(unexpect_t, std::initializer_list<U> il,
                              Args&&... args)
      : base_type(unexpect, il, std::forward<Args>(args)...) {}

  ~expected() = default;

  expected& operator=(const expected&) = default;
  expected& operator=(expected&&) = default;

  // template <class U = T> expected& operator=(U&& v);
  // template <class G = E> expected& operator=(const unexpected<G>&);
  // template <class G = E> expected& operator=(unexpected<G>&&);

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<T, Args&&...>>* = nullptr>
  T& emplace(Args&&... args) {
    if (this->has_val_) {
      this->val_ = T(std::forward<Args>(args)...); // This can throw.
    } else if constexpr (std::is_nothrow_constructible_v<T, Args&&...>) {
      this->destroy(unexpect);
      this->construct(std::in_place, std::forward<Args>(args)...);
    } else if constexpr (std::is_nothrow_move_constructible_v<T>) {
      T tmp(std::forward<Args>(args)...); // This can throw.
      this->destroy(unexpect);
      this->construct(std::in_place, std::move(tmp));
    } else {
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

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible_v<
                T, std::initializer_list<U>, Args&&...>>* = nullptr>
  T& emplace(std::initializer_list<U> il, Args&&... args) {
    if (this->has_val_) {
      this->val_ = T(il, std::forward<Args>(args)...); // This can throw.
    } else if constexpr (std::is_nothrow_constructible_v<
                             T, std::initializer_list<U>, Args&&...>) {
      this->destroy(unexpect);
      this->construct(std::in_place, il, std::forward<Args>(args)...);
    } else if constexpr (std::is_nothrow_move_constructible_v<T>) {
      T tmp(il, std::forward<Args>(args)...); // This can throw.
      this->destroy(unexpect);
      this->construct(std::in_place, std::move(tmp));
    } else {
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

  constexpr const T* operator->() const { return std::addressof(this->val_); }
  constexpr T* operator->() { return std::addressof(this->val_); }

  constexpr const T& operator*() const& { return this->val_; }
  constexpr T& operator*() & { return this->val_; }
  constexpr const T&& operator*() const&& { return std::move(this->val_); }
  constexpr T&& operator*() && { return std::move(this->val_); }

  constexpr explicit operator bool() const noexcept { return this->has_val_; }
  constexpr bool has_value() const noexcept { return this->has_val_; }

  constexpr const T& value() const& {
    if (!this->has_val_)
      throw bad_expected_access(this->unexpect_.value());
    return this->val_;
  }

  constexpr T& value() & {
    if (!this->has_val_)
      throw bad_expected_access(this->unexpect_.value());
    return this->val_;
  }

  constexpr const T&& value() const&& {
    if (!this->has_val_)
      throw bad_expected_access(std::move(this->unexpect_.value()));
    return std::move(this->val_);
  }

  constexpr T&& value() && {
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

  template <class U> constexpr T value_or(U&& v) const& {
    static_assert(std::is_copy_constructible_v<T> &&
                  std::is_convertible_v<U&&, T>);
    return this->has_val_ ? this->val_ : static_cast<T>(std::forward<U>(v));
  }

  template <class U> constexpr T value_or(U&& v) && {
    static_assert(std::is_move_constructible_v<T> &&
                  std::is_convertible_v<U&&, T>);
    return this->has_val_ ? std::move(this->val_)
                          : static_cast<T>(std::forward<U>(v));
  }

  // void swap(expected& other) noexcept(see below);
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

// template <class T, class E>
// void swap(expected<T, E>& x, expected<T, E>& y) noexcept(noexcept(x.swap(y)))

} // namespace bc::exp

#endif
