cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    regex-machine
    VERSION 0.1.0
    DESCRIPTION "A header-only regex library"
    HOMEPAGE_URL "https://github.com/lkk7/regex-machine"
    LANGUAGES NONE
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)


add_library(regex-machine_regex-machine INTERFACE)
add_library(regex-machine::regex-machine ALIAS regex-machine_regex-machine)

set_property(
    TARGET regex-machine_regex-machine PROPERTY
    EXPORT_NAME regex-machine
)

target_include_directories(
    regex-machine_regex-machine ${warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(regex-machine_regex-machine INTERFACE cxx_std_20)

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

if(NOT regex-machine_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of regex-machine"
  )
endif()

include(cmake/dev-mode.cmake)
