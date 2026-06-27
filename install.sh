#!/bin/bash
set -e

CURDIR=$(dirname $(readlink -f $0))

main()
{
    # 先卸载现有的包
    echo "Uninstalling old packages..."
    pip uninstall aclruntime ais_bench -y || true

    # 重新安装
    echo "Installing aclruntime..."
    pip install $CURDIR/aclruntime || { echo "pip install aclruntime failed"; exit 1; }

    echo "Installing ais_bench..."
    pip install $CURDIR/ais_bench || { echo "pip install ais_bench failed"; exit 1; }

    echo "All packages installed successfully!"
}

main "$@"
