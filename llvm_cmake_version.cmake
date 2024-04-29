# From SerenityOS Meta/CMake/cmake_version.cmake

set(ok 0)
if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.20.0)
    set(ok 1)
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -E echo "${ok}")
