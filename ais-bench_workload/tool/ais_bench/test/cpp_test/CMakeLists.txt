project(backend VERSION 1.0.0 LANGUAGES CXX C)
cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(cpython MODULE REQUIRED)
find_package(gtest MODULE REQUIRED)
find_package(mockcpp MODULE REQUIRED)

add_executable(backend_test)
target_link_libraries(backend_test PRIVATE ${gtest_LIBRARIES})
target_link_libraries(backend_test PRIVATE ${mockcpp_LIBRARIES})
target_link_libraries(backend_test PRIVATE _backend_c)

target_include_directories(backend_test PRIVATE $ENV{PROJECT_ROOT_PATH}/backend)
target_include_directories(backend_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(backend_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/mock)

target_compile_definitions(backend_test PRIVATE __RESOURCES_PATH__="${CMAKE_CURRENT_SOURCE_DIR}/../resources")

file(GLOB_RECURSE SOURCES "*.cpp")
target_sources(backend_test PUBLIC ${SOURCES})
