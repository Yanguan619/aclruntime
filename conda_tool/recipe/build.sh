#!/bin/bash

# architecture support x86_64 or aarch64
ARCH_TYPE=$(uname -m)
if [ "$ARCH_TYPE" != "x86_64" ] && [ "$ARCH_TYPE" != "aarch64" ]; then
    echo "Error the system architecture is $ARCH_TYPE, is not in support lsit [x86_64, aarch64]"
    exit 1
fi

mkdir -p $PREFIX/Ascend
cp -r $SRC_DIR/*   $PREFIX/Ascend
