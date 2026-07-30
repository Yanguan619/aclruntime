#!/bin/bash
set -e

CURDIR=$(dirname $(readlink -f $0))

check_deps()
{
    if ! command -v zlib.h &>/dev/null && ! [ -f /usr/include/zlib.h ] && ! [ -f /usr/local/include/zlib.h ]; then
        echo "zlib development headers not found, installing..."
        if command -v apt-get &>/dev/null; then
            sudo apt-get install -y zlib1g-dev
        elif command -v dnf &>/dev/null; then
            sudo dnf install -y zlib-devel gcc-c++
        elif command -v yum &>/dev/null; then
            sudo yum install -y zlib-devel
        elif command -v zypper &>/dev/null; then
            sudo zypper install -y zlib-devel
        else
            echo "Warning: Could not install zlib-devel automatically. Please install it manually."
        fi
    fi
}

main()
{
    check_deps
    pip uninstall aclruntime ais_bench -y || true
    pip install -r $CURDIR/requirements.txt
}

main "$@"
