#!/bin/bash

osdenpendency=(
    "gcc --version"
    "g++ --version"
    "cmake --version"
    "python --version"
)

exitcode=0
for cmd in "${osdenpendency[@]}"; do
    echo =============================================
    if ! $cmd; then
        echo "Command failed: ${cmd}" >&2
        exitcode=1
    fi
done

exit $exitcode