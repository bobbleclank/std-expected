#ifndef TEST_UNION_ACCESS_H
#define TEST_UNION_ACCESS_H

inline auto& val(auto& b) {
  return b.val_;
}

inline auto& unexpect_value(auto& b) {
  return b.unexpect_.value();
}

inline auto& uninit(auto& b) {
  return b.uninit_;
}

inline auto& dummy(auto& b) {
  return b.dummy_;
}

#endif
