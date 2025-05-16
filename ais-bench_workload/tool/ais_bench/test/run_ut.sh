#!/bin/bash
CUR_DIR=$(dirname $(readlink -f $0))
TOP_DIR=${CUR_DIR}/../..
MSIT_TEST=${TOP_DIR}/output/release/test/UT/csrc_ut/msit_test
TEST_DIR=${TOP_DIR}/test/UT
SRC_DIR=${TOP_DIR}/

run_ut_cpp() {
    echo "[INFO] Start compiling msit_test..."
    ARCH_TYPE=$(uname -m)
    PYTHON_VERSION=$(python3 -c 'import platform; print(".".join(platform.python_version_tuple()[:2]))')
    BUILD_SCRIPT=${TOP_DIR}/build.sh
    if [[ ! -f "${BUILD_SCRIPT}" ]]; then
        echo "[ERROR] build.sh not found at ${BUILD_SCRIPT}"
        exit 1
    fi
    bash "${BUILD_SCRIPT}" \
        --release \
        -t \
        -a "${ARCH_TYPE}" \
        -v "${PYTHON_VERSION}" \
        -j 16 \

    if [[ -x "${MSIT_TEST}" ]]; then
        echo "[INFO] Running C++ unit test binary: ${MSIT_TEST}"
        ${MSIT_TEST}
    else
        echo "[ERROR] msit_test binary not found or not executable at ${MSIT_TEST}"
        exit 1
    fi
}

install_pytest() {
    if ! pip show pytest &> /dev/null; then
        echo "pytest not found, trying to install..."
        pip install pytest
    fi

    if ! pip show pytest-cov &> /dev/null; then
        echo "pytest-cov not found, trying to install..."
        pip install pytest-cov
    fi
}

run_ut_py() {
    install_pytest
    export PYTHONPATH=${SRC_DIR}:${PYTHONPATH}
    python3 run_ut.py
}

main() {
    run_ut_cpp
    cd ${TEST_DIR} && run_ut_py
}

main $@
