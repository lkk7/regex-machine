if(PROJECT_IS_TOP_LEVEL)
  option(regex-machine_DEVELOPER_MODE "Enable developer mode" OFF)
endif()

set(warning_guard "")
if(NOT PROJECT_IS_TOP_LEVEL)
  option(
      regex-machine_INCLUDES_WITH_SYSTEM
      "Use SYSTEM modifier for regex-machine's includes, disabling warnings"
      ON
  )
  mark_as_advanced(regex-machine_INCLUDES_WITH_SYSTEM)
  if(regex-machine_INCLUDES_WITH_SYSTEM)
    set(warning_guard SYSTEM)
  endif()
endif()
