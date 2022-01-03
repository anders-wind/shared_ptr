include(cmake/folders.cmake)

include(CTest)

option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

option(BUILD_BENCHMARK "Build benchmarks" OFF)
if(BUILD_BENCHMARK)
  add_subdirectory(benchmark)
endif()

option(ENABLE_COVERAGE "Enable coverage support separate from CTest's" OFF)
if(ENABLE_COVERAGE)
  include(cmake/coverage.cmake)
endif()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  include(cmake/open-cpp-coverage.cmake OPTIONAL)
endif()

include(cmake/lint-targets.cmake)
include(cmake/spell-targets.cmake)

add_folders(Project)
