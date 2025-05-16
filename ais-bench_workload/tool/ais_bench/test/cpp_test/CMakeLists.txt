project(msit VERSION 1.0.0 LANGUAGES CXX C)
cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(cpython MODULE REQUIRED)
find_package(gtest MODULE REQUIRED)
find_package(mockcpp MODULE REQUIRED)
find_package(nlohmannjson MODULE REQUIRED)

add_executable(msit_test)
target_link_libraries(msit_test PRIVATE ${gtest_LIBRARIES})
target_link_libraries(msit_test PRIVATE ${mockcpp_LIBRARIES})
target_link_libraries(msit_test PRIVATE msit_c)

target_include_directories(msit_test PRIVATE $ENV{PROJECT_ROOT_PATH}/msit/csrc)
target_include_directories(msit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(msit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/mock)

target_compile_definitions(msit_test PRIVATE __RESOURCES_PATH__="${CMAKE_CURRENT_SOURCE_DIR}/../resources")

file(GLOB_RECURSE SOURCES "*.cpp")
target_sources(msit_test PUBLIC ${SOURCES})
