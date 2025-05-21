#!/bin/bash

set -e

BUILD_PATH=$(pwd)

BUILD_ARGS=$(getopt -o ha:v:j:ft --long help,release,debug,arch:python-version:,jobs:,force-rebuild,local,test-cases -- "$@")
eval set -- "${BUILD_ARGS}"

ARCH_TYPE=$(uname -m)
BUILD_TYPE=release
CONCURRENT_JOBS=16
BUILD_TEST_CASE=True
USE_LOCAL_FIRST=False
PYTHON_VERSION=""

HELP_DOC=$(cat << EOF
Usage: build.sh [OPTION]...\n
Build the C++ part of aisbench.\n
\n
Arguments:\n
    -a, --arch                    Specify the schema, which generally does not need to be set up.\n
    -j, --jobs                    Specify the number of compilation jobs(default 16).\n
    -f, --force-rebuild           Clean up the cache before building.\n
    -t, --test-cases              Build test cases.\n
        --local                   Prioritize the use of on-premises, third-party resources as dependencies.\n
        --release                 Build the release version(default).\n
        --debug                   Build the debug version.
    -v, --python-version          Specify version of python.
EOF
)

cann_base_path=""
get_cann_path() {
    local set_env_path="${CANN_PATH:-}"

    if [ -z "$set_env_path" ]; then
        set_env_path="${ASCEND_TOOLKIT_HOME:-}"
        if [ -z "$set_env_path" ]; then
            set_env_path="/usr/local/Ascend/ascend-toolkit/latest/"
        else
            set_env_path=$(echo "$set_env_path" | cut -d':' -f1)
        fi
    else
        set_env_path=$(echo "$set_env_path" | cut -d':' -f1)
    fi

    atlas_nnae_path="/usr/local/Ascend/nnae/latest/"
    atlas_toolkit_path="/usr/local/Ascend/ascend-toolkit/latest/"
    hisi_fwk_path="/usr/local/Ascend/"
    check_file_path="runtime/lib64/stub/libascendcl.so"

    if [ -f "$set_env_path/$check_file_path" ]; then
        cann_base_path="$set_env_path"
    elif [ -f "$atlas_nnae_path$check_file_path" ]; then
        cann_base_path="$atlas_nnae_path"
    elif [ -f "$atlas_toolkit_path$check_file_path" ]; then
        cann_base_path="$atlas_toolkit_path"
    elif [ -f "$hisi_fwk_path$check_file_path" ]; then
        cann_base_path="$hisi_fwk_path"
    fi

    if [ -z "$cann_base_path" ]; then
        if [ "$(uname -m)" == "x86_64" ]; then
            check_file_path="runtime/lib64/stub/x86_64/libascendcl.so"
        elif [ "$(uname -m)" == "aarch64" ]; then
            check_file_path="runtime/lib64/stub/aarch64/libascendcl.so"
        fi

        if [ -f "$set_env_path/$check_file_path" ]; then
            cann_base_path="$set_env_path"
        elif [ -f "$atlas_nnae_path$check_file_path" ]; then
            cann_base_path="$atlas_nnae_path"
        elif [ -f "$atlas_toolkit_path$check_file_path" ]; then
            cann_base_path="$atlas_toolkit_path"
        elif [ -f "$hisi_fwk_path$check_file_path" ]; then
            cann_base_path="$hisi_fwk_path"
        fi

        if [ -z "$cann_base_path" ]; then
            echo "error: find no cann path" >&2
            exit 1
        fi
    fi

    echo "find cann path: $cann_bash_path"
}

url="https://github.com/pybind/pybind11.git"
fullname="${BUILD_PATH}/third_party/$(basename "${url}" .git)"
if [[ -e ${fullname} ]]; then
    echo "Source ${fullname} is exists, will not download again."
else
    mkdir ${BUILD_PATH}/third_party
    cd ${BUILD_PATH}/third_party
    git clone ${url}
fi

while true; do
    case "$1" in
        -h | --help)
            echo -e ${HELP_DOC}
            exit 0 ;;
        -a | --arch)
            ARCH_TYPE="$2" ; shift 2 ;;
        -v | --python-version)
            PYTHON_VERSION="$2" ; shift 2 ;;
        --release)
            BUILD_TYPE=release ; shift ;;
        --debug)
            BUILD_TYPE=debug ; shift ;;
        -j | --jobs)
            CONCURRENT_JOBS="$2"  ; shift 2 ;;
        --local)
            USE_LOCAL_FIRST=True ; shift ;;
        -f | --force-rebuild)
            rm -rf "${BUILD_PATH}/lib" "${BUILD_PATH}/output" "${BUILD_PATH}/backend/lib/_backend_c.so"
            shift ;;
        -t | --test-cases)
            BUILD_TEST_CASE=True ; shift ;;
        --)
            shift ; break ;;
        *)
            echo "Unknow argument $1"
            exit 1 ;;
    esac
done

BUILD_OUTPUT_PATH=${BUILD_PATH}/output/${BUILD_TYPE}

get_cann_path

cmake -B ${BUILD_OUTPUT_PATH} -S . -DARCH_TYPE=${ARCH_TYPE} -DBUILD_TYPE=${BUILD_TYPE} \
                                   -DUSE_LOCAL_FIRST=${USE_LOCAL_FIRST} -DBUILD_TEST_CASE=${BUILD_TEST_CASE} \
                                   -DPYTHON_VERSION=${PYTHON_VERSION} -DCANN_BASE_PATH="$cann_base_path"
cd ${BUILD_OUTPUT_PATH}
make -j${CONCURRENT_JOBS}

if [[ ! -e ${BUILD_OUTPUT_PATH}/backend/lib_backend_c.so ]]; then
    echo "Failed to build lib_backend_c.so."
    exit 1
fi

if [[ ! -e ${BUILD_PATH}/backend/lib ]]; then
    mkdir ${BUILD_PATH}/backend/lib
fi

cp ${BUILD_OUTPUT_PATH}/backend/lib_backend_c.so ${BUILD_PATH}/backend/lib/_backend_c.so
