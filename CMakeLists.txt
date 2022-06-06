cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    shared_ptr
    VERSION 0.1.1
    DESCRIPTION "Shared pointer experiments"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)
include(cmake/CPM.cmake)


# ---- Dependencies ----

find_package(Threads REQUIRED)
CPMAddPackage("gh:martinus/robin-hood-hashing#3.11.5")

# ---- Declare library ----

add_library(shared_ptr_shared_ptr INTERFACE)
add_library(wind::shared_ptr ALIAS shared_ptr_shared_ptr)

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

target_link_libraries(shared_ptr_shared_ptr INTERFACE robin_hood::robin_hood Threads::Threads)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
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
