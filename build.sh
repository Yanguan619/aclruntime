#!/bin/bash
set -e

CURDIR=$(dirname $(readlink -f $0))
NATIVE_ARCH=$(uname -m)

build_aclruntime()
{
    local arch=$1
    rm -rf $CURDIR/aclruntime/build $CURDIR/aclruntime/dist

    if [ "$arch" != "$NATIVE_ARCH" ]; then
        local cross_cc=$(command -v "${arch}-linux-gnu-gcc" 2>/dev/null || true)
        local cross_cxx=$(command -v "${arch}-linux-gnu-g++" 2>/dev/null || true)

        if [ -z "$cross_cc" ] || [ -z "$cross_cxx" ]; then
            echo "cross-compiler not found for $arch, installing..."
            case "$arch" in
                x86_64)  sudo apt install -y gcc-x86-64-linux-gnu g++-x86-64-linux-gnu ;;
                aarch64) sudo apt install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu ;;
            esac
            cross_cc=$(command -v "${arch}-linux-gnu-gcc")
            cross_cxx=$(command -v "${arch}-linux-gnu-g++")
        fi

        export CMAKE_ARGS="-DCMAKE_C_COMPILER=$cross_cc -DCMAKE_CXX_COMPILER=$cross_cxx -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=$arch"
        (cd $CURDIR/aclruntime && uv build --wheel) || return 1
        unset CMAKE_ARGS
    else
        (cd $CURDIR/aclruntime && uv build --wheel) || return 1
    fi
}

main()
{
    rm -rf $CURDIR/aclruntime/.egg-info $CURDIR/aclruntime/build $CURDIR/aclruntime/dist
    rm -rf $CURDIR/ais_bench/.egg-info $CURDIR/ais_bench/build $CURDIR/ais_bench/dist

    # ais_bench is pure Python, arch-independent
    (cd $CURDIR/ais_bench && uv build --wheel) || { echo "uv build ais_bench failed"; exit 1; }

    # aclruntime: build for all supported architectures
    for arch in x86_64 aarch64; do
        build_aclruntime $arch || { echo "uv build aclruntime ($arch) failed"; exit 1; }
    done
}

main "$@"
