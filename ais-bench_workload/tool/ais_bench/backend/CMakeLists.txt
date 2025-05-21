cmake_minimum_required(VERSION 3.14)
cmake_policy(SET CMP0048 NEW)

project(backend VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(_backend_c SHARED)

if(DEFINED CANN_BASE_PATH)
    message(STATUS "Found cann path: ${CANN_BASE_PATH}")
else()
    message(ERROR "Cannot find cann path")
endif()

find_package(cpython MODULE REQUIRED)

target_include_directories(_backend_c PRIVATE ${CANN_BASE_PATH}/runtime/include)
target_include_directories(_backend_c PRIVATE ${CANN_BASE_PATH}/../third_party/pybind11/include)
target_include_directories(_backend_c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/python/include)
target_include_directories(_backend_c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/base/include)
target_include_directories(_backend_c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/base/include/Base/ModelInfer)

target_compile_options(_backend_c PRIVATE "-Wall")
target_compile_options(_backend_c PRIVATE "-fPIC")
target_compile_options(_backend_c PRIVATE "-fstack-protector-all")
target_compile_options(_backend_c PRIVATE "-ftrapv")
target_compile_options(_backend_c PRIVATE "-fstack-check")
target_compile_options(_backend_c PRIVATE "-D_FORTIFY_SOURCE=2")

target_link_options(_backend_c PRIVATE "-Wl,-z,relro")
target_link_options(_backend_c PRIVATE "-Wl,-z,now")
target_link_options(_backend_c PRIVATE "-Wl,-z,noexecstack")
target_link_options(_backend_c PRIVATE "-Wl,--disable-new-dtags")
target_link_options(_backend_c PRIVATE "-s")

target_link_libraries(_backend_c PUBLIC dl)
target_link_libraries(_backend_c PUBLIC pthread)
target_link_libraries(_backend_c PUBLIC ${cpython_LIBRARIES})
target_link_libraries(_backend_c PRIVATE ${CANN_BASE_PATH}/acllib/lib64/stub/libascendcl.so)
target_link_libraries(_backend_c PRIVATE ${CANN_BASE_PATH}/acllib/lib64/stub/libacl_dvpp.so)

target_compile_definitions(_backend_c PRIVATE ENABLE_DVPP_INTERFACE=1)
target_compile_definitions(_backend_c PRIVATE COMPILE_PYTHON_MODULE=1)

if(DEFINED BUILD_TYPE AND "${BUILD_TYPE}" STREQUAL "debug")
    target_compile_options(_backend_c PRIVATE "-O0")
    target_compile_options(_backend_c PRIVATE "-g")
    target_compile_definitions(_backend_c PRIVATE __DEBUG__)
else()
    target_compile_options(_backend_c PRIVATE "-O2")
endif()

file(GLOB_RECURSE SOURCES "*.cpp" "!cpp_interface/*")
file(GLOB_RECURSE REMOVE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/cpp_interface/*.cpp")
list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})

target_sources(_backend_c PRIVATE ${SOURCES})

install(TARGETS _backend_c LIBRARY DESTINATION lib)
