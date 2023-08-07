cmake_minimum_required(VERSION 3.14)

project(regex-machineTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)


if(PROJECT_IS_TOP_LEVEL)
  find_package(regex-machine REQUIRED)
  enable_testing()
endif()

add_executable(regex-machine_test source/regex-machine_test.cpp)
target_link_libraries(regex-machine_test PRIVATE regex-machine::regex-machine)
target_compile_features(regex-machine_test PRIVATE cxx_std_20)

add_test(NAME regex-machine_test COMMAND regex-machine_test)


add_folders(Test)
