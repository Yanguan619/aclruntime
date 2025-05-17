project(backend VERSION 1.0.0 LANGUAGES CXX C)
cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(cpython MODULE REQUIRED)
find_package(nlohmannjson MODULE REQUIRED)
find_package(protobuf MODULE REQUIRED)

add_library(_backend_c SHARED)

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
target_link_libraries(_backend_c PUBLIC ${protobuf_LIBRARIES})

if(DEFINED BUILD_TYPE AND "${BUILD_TYPE}" STREQUAL "debug")
    target_compile_options(_backend_c PRIVATE "-O0")
    target_compile_options(_backend_c PRIVATE "-g")
    target_compile_definitions(_backend_c PRIVATE __DEBUG__)
else()
    target_compile_options(_backend_c PRIVATE "-O2")
endif()

target_include_directories(_backend_c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SOURCES "*.cpp" "!cpp_interface/*")
target_sources(_backend_c PRIVATE ${SOURCES})

install(TARGETS _backend_c LIBRARY DESTINATION lib)
