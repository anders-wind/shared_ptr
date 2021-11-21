cmake_minimum_required(VERSION 3.14)

project(shared_ptrTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/windows-set-path.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(shared_ptr REQUIRED)
  enable_testing()
endif()

CPMAddPackage("gh:onqtam/doctest#2.4.6")

add_executable(shared_ptr_test 
  source/main_test.cpp 
  source/shared_ptr_test.cpp 
  source/thread_local_value_test.cpp)

target_link_libraries(shared_ptr_test 
  PRIVATE shared_ptr::shared_ptr doctest::doctest)

target_compile_features(shared_ptr_test PRIVATE cxx_std_20)

add_test(NAME shared_ptr_test COMMAND shared_ptr_test)

add_folders(Test)
