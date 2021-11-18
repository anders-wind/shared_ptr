cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    shared_ptr
    VERSION 0.1.0
    DESCRIPTION "Shared pointer experiments"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES NONE
)

docs_early_return()

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(shared_ptr_shared_ptr INTERFACE)
add_library(shared_ptr::shared_ptr ALIAS shared_ptr_shared_ptr)

set_property(
    TARGET shared_ptr_shared_ptr PROPERTY
    EXPORT_NAME shared_ptr
)

target_include_directories(
    shared_ptr_shared_ptr ${warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(shared_ptr_shared_ptr INTERFACE cxx_std_20)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${shared_ptr_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(example)
  endif()
endif()

# ---- Developer mode ----

if(NOT shared_ptr_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of shared_ptr"
  )
endif()

include(cmake/dev-mode.cmake)
