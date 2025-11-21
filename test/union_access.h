#ifndef TEST_UNION_ACCESS_H
#define TEST_UNION_ACCESS_H

// NOLINTBEGIN(*-pro-type-union-access): Tagged union

inline auto& val(auto& b) {
  return b.val_;
}

inline auto& err(auto& b) {
  return b.unexpect_.value();
}

inline auto& uninit(auto& b) {
  return b.uninit_;
}

inline auto& dummy(auto& b) {
  return b.dummy_;
}

inline bool has_val(auto& b) {
  return b.has_val_;
}

// NOLINTEND(*-pro-type-union-access): Tagged union

#endif
