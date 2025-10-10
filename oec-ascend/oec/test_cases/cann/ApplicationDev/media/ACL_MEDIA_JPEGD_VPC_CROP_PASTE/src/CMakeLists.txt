# Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.

# CMake lowest version requirement
cmake_minimum_required(VERSION 3.5.1)

# project information
project(ACL_DVPP_RESNET50)

# Compile options
add_compile_options(-std=c++11)

add_definitions(-DENABLE_DVPP_INTERFACE)

# Specify target generation path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "../")
set(CMAKE_CXX_FLAGS_DEBUG "-fPIC -O0 -g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-fPIC -O2 -Wall")

set(INC_PATH $ENV{ASCEND_HOME_PATH})
message(STATUS "env INC_PATH: ${INC_PATH}")
set(LIB_PATH "$ENV{ASCEND_HOME_PATH}/lib64")
message(STATUS "env LIB_PATH: ${LIB_PATH}")

# Header path
include_directories(
    ${INC_PATH}/runtime/include/
    ../inc/
)

# add host lib path
link_directories(
    ${LIB_PATH}
)

add_executable(main
        utils.cpp
        dvpp_process.cpp
        sample_process.cpp
        main.cpp)

if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(main
        libascendcl libacl_dvpp)
else ()
    target_link_libraries(main
        ascendcl acl_dvpp stdc++)
endif ()

install(TARGETS main DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
