#ifndef TEST_COMP_H
#define TEST_COMP_H

struct Equal_to {
  template <class T, class U>
  constexpr bool operator()(const T& lhs, const U& rhs) const {
    return lhs == rhs;
  }
};

struct Not_equal_to {
  template <class T, class U>
  constexpr bool operator()(const T& lhs, const U& rhs) const {
    return lhs != rhs;
  }
};

#endif
