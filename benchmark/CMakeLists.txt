cmake_minimum_required(VERSION 3.14)

project(shared_ptrBenchmarks LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/windows-set-path.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(shared_ptr REQUIRED)
endif()

CPMAddPackage(
  NAME benchmark
  GITHUB_REPOSITORY google/benchmark
  VERSION 1.6.0
  OPTIONS "BENCHMARK_ENABLE_TESTING Off" "BENCHMARK_INSTALL_DOCS Off" "BENCHMARK_ENABLE_INSTALL Off"
)

add_executable(shared_ptr_benchmark source/shared_ptr_benchmark.cpp)

target_link_libraries(shared_ptr_benchmark 
  PRIVATE shared_ptr::shared_ptr benchmark::benchmark)

target_compile_features(shared_ptr_benchmark PRIVATE cxx_std_20)

add_folders(Benchmark)
