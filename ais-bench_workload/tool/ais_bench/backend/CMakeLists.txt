cmake_minimum_required(VERSION 3.14)
cmake_policy(SET CMP0048 NEW)
project(ais_bench_backend VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(cpython MODULE REQUIRED)
find_package(nlohmannjson MODULE REQUIRED)
find_package(protobuf MODULE REQUIRED)

add_library(ais_bench_backend_c SHARED)

add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=0)
target_compile_definitions(ais_bench_backend_c PRIVATE _GLIBCXX_USE_CXX11_ABI=0)

# 安全编译选项
target_compile_options(ais_bench_backend_c PRIVATE "-Wall")
target_compile_options(ais_bench_backend_c PRIVATE "-fPIC")
target_compile_options(ais_bench_backend_c PRIVATE "-fstack-protector-all")
target_compile_options(ais_bench_backend_c PRIVATE "-ftrapv")
target_compile_options(ais_bench_backend_c PRIVATE "-D_FORTIFY_SOURCE=2")

target_link_options(ais_bench_backend_c PRIVATE "-Wl,-z,relor")
target_link_options(ais_bench_backend_c PRIVATE "-Wl,-z,now")
target_link_options(ais_bench_backend_c PRIVATE "-Wl,-z,noexecstack")
target_link_options(ais_bench_backend_c PRIVATE "-Wl,--disable-new-dtags")
target_link_options(ais_bench_backend_c PRIVATE "-s")

target_link_libraries(ais_bench_backend_c PUBLIC dl)
target_link_libraries(ais_bench_backend_c PUBLIC pthread)
target_link_libraries(ais_bench_backend_c PUBLIC ${cpython_LIBRARIES})

if(DEFINED BUILD_TYPE AND "${BUILD_TYPE}" STREQUAL "debug")
    target_compile_options(ais_bench_backend_c PRIVATE "-O0")
    target_compile_options(ais_bench_backend_c PRIVATE "-g")
    target_compile_definitions(ais_bench_backend_c PRIVATE __DEBUG__)
else()
    target_compile_options(ais_bench_backend_c PRIVATE "-O2")
endif()

target_include_directories(ais_bench_backend_c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SOURCES "*.cpp")
target_sources(ais_bench_backend_c PRIVATE ${SOURCES})

install(TARGETS ais_bench_backend_c LIBRARY DESTINATION lib)
