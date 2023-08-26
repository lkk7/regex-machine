cmake_minimum_required(VERSION 3.14)

project(regex-machineTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)


if(PROJECT_IS_TOP_LEVEL)
  find_package(regex-machine REQUIRED)
  enable_testing()
endif()

find_package(Catch2 REQUIRED)
include(Catch)

add_executable(regex_machine_test
  source/nfa_creation_test.cpp
  source/nfa_test.cpp
  source/parser_test.cpp
  source/regex_machine_test.cpp
  source/scanner_test.cpp
)
target_link_libraries(regex_machine_test PRIVATE regex-machine::regex-machine Catch2::Catch2WithMain)
target_compile_features(regex_machine_test PRIVATE cxx_std_20)

catch_discover_tests(regex_machine_test)

add_folders(Test)
