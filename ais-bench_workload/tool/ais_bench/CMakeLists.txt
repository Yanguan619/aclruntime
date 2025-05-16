project(inference_tools)
cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

execute_process(
    COMMAND uname -m
    OUTPUT_VARIABLE machine_arch
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (DEFINED ARCH_TYPE AND NOT "${ARCH_TYPE}" STREQUAL "${machine_arch}")
    message(FATAL_ERROR "Cross-compilation is not supported currently. (compile ${ARCH_TYPE} on ${machine_arch})")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(ENV{PROJECT_ROOT_PATH} "${CMAKE_SOURCE_DIR}")
include("${CMAKE_SOURCE_DIR}/cmake/utils.cmake")
add_subdirectory(msit)

if (DEFINED BUILD_TEST_CASE AND "${BUILD_TEST_CASE}" STREQUAL "True")
    add_subdirectory(test/UT)
endif()
