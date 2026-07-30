cmake_minimum_required(VERSION 3.14)
project(aclruntime VERSION 0.0.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Detect platform
if(WIN32)
    set(_is_linux FALSE)
    set(_is_win TRUE)
else()
    set(_is_linux TRUE)
    set(_is_win FALSE)
endif()

# log
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)


# --- CANN path discovery ---
if(DEFINED CANN_BASE_PATH)
    set(_cann_base "${CANN_BASE_PATH}")
elseif(DEFINED ENV{CANN_PATH} AND NOT "$ENV{CANN_PATH}" STREQUAL "")
    set(_cann_base "$ENV{CANN_PATH}")
elseif(DEFINED ENV{ASCEND_TOOLKIT_HOME} AND NOT "$ENV{ASCEND_TOOLKIT_HOME}" STREQUAL "")
    set(_cann_base "$ENV{ASCEND_TOOLKIT_HOME}")
elseif(DEFINED ENV{ASCEND_HOME_PATH} AND NOT "$ENV{ASCEND_HOME_PATH}" STREQUAL "")
    set(_cann_base "$ENV{ASCEND_HOME_PATH}")
else()
    if(_is_linux)
        foreach(_p /usr/local/Ascend/cann/ /usr/local/Ascend/ascend-toolkit/latest/
                  /usr/local/Ascend/nnae/latest/ /usr/local/Ascend/)
            if(EXISTS "${_p}")
                set(_cann_base "${_p}")
                break()
            endif()
        endforeach()
    elseif(_is_win)
        foreach(_p "C:/Program Files/Ascend/cann/" "C:/Program Files/Ascend/ascend-toolkit/latest/")
            if(EXISTS "${_p}")
                set(_cann_base "${_p}")
                break()
            endif()
        endforeach()
    endif()
endif()

if(NOT _cann_base)
    message(FATAL_ERROR "Cannot find CANN path. Set CANN_BASE_PATH, CANN_PATH, or ASCEND_TOOLKIT_HOME.")
endif()
message(STATUS "CANN base: ${_cann_base}")

if(_is_linux)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
        set(_sub_paths "runtime/lib64/stub/x86_64/" "x86_64-linux/lib64/")
        set(_inc_subs "runtime/include/" "x86_64-linux/include/")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(_sub_paths "runtime/lib64/stub/aarch64/" "aarch64-linux/lib64/")
        set(_inc_subs "runtime/include/" "aarch64-linux/include/")
    else()
        message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
elseif(_is_win)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
        set(_sub_paths "runtime/lib64/stub/x86_64/"
                       "x86_64-linux/lib64/"
                       "lib64/"
                       "lib/")
        set(_inc_subs "runtime/include/"
                      "x86_64-linux/include/"
                      "include/")
    else()
        message(FATAL_ERROR "Unsupported Windows architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

set(_cann_lib "")
set(_cann_inc "")
foreach(_lib_sub ${_sub_paths})
    foreach(_inc_sub ${_inc_subs})
        if(EXISTS "${_cann_base}/${_lib_sub}" AND EXISTS "${_cann_base}/${_inc_sub}")
            set(_cann_lib "${_cann_base}/${_lib_sub}")
            set(_cann_inc "${_cann_base}/${_inc_sub}")
            break()
        endif()
    endforeach()
    if(_cann_lib)
        break()
    endif()
endforeach()

if(NOT _cann_lib)
    # Fallback: find any lib/ and include/ pair
    file(GLOB_RECURSE _cann_candidates "${_cann_base}/libascendcl.*")
    if(_cann_candidates)
        get_filename_component(_cann_lib "${_cann_candidates}" DIRECTORY)
        set(_cann_inc "${_cann_base}/include")
        message(STATUS "CANN lib (fallback): ${_cann_lib}")
    else()
        message(FATAL_ERROR "Cannot find CANN lib/include under ${_cann_base}")
    endif()
endif()
message(STATUS "CANN lib: ${_cann_lib}")
message(STATUS "CANN inc: ${_cann_inc}")

# --- pybind11 ---
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 REQUIRED)

# --- Sources ---
file(GLOB_RECURSE _sources "src/*.cpp")

pybind11_add_module(aclruntime MODULE ${_sources})

target_include_directories(aclruntime SYSTEM PRIVATE
    "${_cann_inc}"
    "include"
)

set_property(TARGET aclruntime PROPERTY INTERPROCEDURAL_OPTIMIZATION OFF)

if(_is_linux)
    target_compile_options(aclruntime PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -fPIC -fstack-protector-all -ftrapv -fstack-clash-protection

    )
    target_link_options(aclruntime PRIVATE
        -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack
        -Wl,--disable-new-dtags
    )
elseif(_is_win)
    target_compile_options(aclruntime PRIVATE
        /W4 /utf-8
    )
endif()

target_link_libraries(aclruntime PRIVATE
    "${_cann_lib}/libascendcl${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${_cann_lib}/libacl_dvpp${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${_cann_lib}/libacl_cblas${CMAKE_SHARED_LIBRARY_SUFFIX}"
    fmt::fmt
    spdlog::spdlog
)

target_compile_definitions(aclruntime PRIVATE
    SPDLOG_FMT_EXTERNAL
    ENABLE_DVPP_INTERFACE=1
    COMPILE_PYTHON_MODULE=1
)

install(TARGETS aclruntime LIBRARY DESTINATION .)
install(FILES "${CMAKE_SOURCE_DIR}/aclruntime.pyi" DESTINATION . OPTIONAL)

# ── Optional: generate .pyi stubs via gen_stubs.py ──
find_package(Python COMPONENTS Interpreter QUIET)
add_custom_command(TARGET aclruntime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH="$<TARGET_FILE_DIR:aclruntime>"
        ${Python_EXECUTABLE} "${CMAKE_SOURCE_DIR}/tools/gen_stubs.py" || true
    WORKING_DIRECTORY $<TARGET_FILE_DIR:aclruntime>
    COMMENT "Generating pybind11 type stubs (if pybind11-stubgen + CANN available)"
)
