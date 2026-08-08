add_library(bcexpected_warnings INTERFACE)
target_compile_options(bcexpected_warnings
  INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)
