#!/bin/bash
CUR_DIR=$(dirname $(readlink -f $0))
TOP_DIR=${CUR_DIR}/../..
CPP_TEST=${TOP_DIR}/output/release/test/cpp_test/backend_test
TEST_DIR=${TOP_DIR}/test/cpp_test
SRC_DIR=${TOP_DIR}/

run_ut_cpp() {
    echo "[INFO] Start compiling backend_test..."
    ARCH_TYPE=$(uname -m)
    PYTHON_VERSION=$(python3 -c 'import platform; print(".".join(platform.python_version_tuple()[:2]))')

    if [[ -x "${CPP_TEST}" ]]; then
        echo "[INFO] Running C++ unit test binary: ${CPP_TEST}"
        ${CPP_TEST}
    else
        echo "[ERROR] backend_test binary not found or not executable at ${CPP_TEST}"
        exit 1
    fi
}

main() {
    run_ut_cpp
}

main $@
