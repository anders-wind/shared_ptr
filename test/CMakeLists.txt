cmake_minimum_required(VERSION 3.14)

project(shared_ptrTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/windows-set-path.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(shared_ptr REQUIRED)
  enable_testing()
endif()

CPMAddPackage("gh:onqtam/doctest#2.4.7")
set_target_properties(doctest PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:doctest,INTERFACE_INCLUDE_DIRECTORIES>)

add_executable(shared_ptr_test 
  source/main_test.cpp 
  source/local_shared_ptr_test.cpp 
  source/bias_shared_ptr_test.cpp
)

target_link_libraries(shared_ptr_test 
  PRIVATE wind::shared_ptr doctest::doctest)

target_compile_features(shared_ptr_test PRIVATE cxx_std_20)

add_test(NAME shared_ptr_test COMMAND shared_ptr_test)

add_folders(Test)
