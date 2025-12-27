add_library(bcexpected INTERFACE)
target_sources(bcexpected
  INTERFACE
    FILE_SET HEADERS
    FILES
      bc/expected.h
)
