#!/bin/bash

# 检查glibc版本是否大于2.17
check_glibc() {
    local required_version="2.17"
    local glibc_version
    
    # 尝试多种方式获取glibc版本
    if command -v ldd >/dev/null 2>&1; then
        glibc_version=$(ldd --version 2>&1 | awk 'NR==1 {print $NF}')
    elif [ -f /lib/x86_64-linux-gnu/libc.so.6 ]; then
        glibc_version=$(/lib/x86_64-linux-gnu/libc.so.6 2>&1 | grep "GNU C Library" | awk '{print $NF}')
    elif [ -f /lib64/libc.so.6 ]; then
        glibc_version=$(/lib64/libc.so.6 2>&1 | grep "GNU C Library" | awk '{print $NF}')
    else
        echo "错误: 无法检测glibc版本 - 请手动安装glibc"
        return 1
    fi

    # 验证版本格式
    if ! [[ $glibc_version =~ ^[0-9]+\.[0-9]+ ]]; then
        echo "错误: 无法解析glibc版本: '$glibc_version'"
        return 1
    fi

    # 版本比较
    if awk -v req="$required_version" -v curr="$glibc_version" 'BEGIN {
        split(req, r, "."); split(curr, c, ".");
        for (i=1; i<=3; i++) {
            if (c[i]+0 < r[i]+0) exit 1;
            if (c[i]+0 > r[i]+0) exit 0;
        }
        exit 0
    }'; then
        echo "通过: glibc版本满足要求 ($glibc_version >= $required_version)"
        return 0
    else
        echo "错误: glibc版本过低 (当前: $glibc_version < 要求: $required_version)"
        return 1
    fi
}

# 检查命令是否存在
check_command() {
    if command -v "$1" >/dev/null 2>&1; then
        echo "通过: $1 命令可用"
        return 0
    else
        echo "错误: $1 命令未找到"
        return 1
    fi
}

# 主检查函数
main() {
    local all_success=0
    # 所有需要检查的命令列表
    local commands=(
        "gcc" "g++" "cmake" "make" "ifconfig"
        "tar" "realpath" "arch" "grep" "sed"
    )
    
    echo "开始依赖检查..."
    echo "=============================="
    
    # 检查glibc版本
    if ! check_glibc; then
        all_success=1
    fi
    
    # 检查必需命令
    for cmd in "${commands[@]}"; do
        if ! check_command "$cmd"; then
            all_success=1
        fi
    done
    
    echo "=============================="
    
    # 返回最终状态
    if [ $all_success -ne 0 ]; then
        echo "依赖检查失败! 请解决以上问题后再运行程序"
        exit 1
    else
        echo "所有依赖检查通过! 可以安全运行程序"
        exit 0
    fi
}

# 执行主函数
main