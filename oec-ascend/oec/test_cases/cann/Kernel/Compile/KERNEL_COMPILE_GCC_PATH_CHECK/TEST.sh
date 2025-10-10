#!/bin/bash

# 使用数组直接存储路径（避免空格问题）
paths=(
    "/usr/lib/gcc/x86_64-linux-gnu/"
    "/usr/lib/gcc/aarch64-linux-gnu"

    "/usr/lib/gcc/x86_64-openEuler-linux"
    "/usr/lib/gcc/aarch64-openEuler-linux"
)

for path in "${paths[@]}"; do
    if [[ -e "$path" ]]; then
        echo "存在路径: $path"
        exit 0
    fi
done

echo "所有路径均不存在"
exit 255