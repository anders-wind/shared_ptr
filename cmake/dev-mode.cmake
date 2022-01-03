include(cmake/folders.cmake)

include(CTest)

option(shared_ptr_BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING AND shared_ptr_BUILD_TESTING)
  add_subdirectory(test)
endif()

option(shared_ptr_BUILD_BENCHMARK "Build benchmarks" OFF)
if(shared_ptr_BUILD_BENCHMARK)
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
