add_library(bcexpected INTERFACE)
target_sources(bcexpected
  INTERFACE
    FILE_SET HEADERS
    FILES
      bc/expected.h
)
target_compile_features(bcexpected
  INTERFACE
    cxx_std_23
)
