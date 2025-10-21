#！/bin/bash

NAME="NAME"
VERSION="VERSION"

# get system architecture
ARCH_TYPE=$(uname -m)

# architecture support x86_64 or aarch64
if [ "$ARCH_TYPE" != "x86_64" ] && [ "$ARCH_TYPE" != "aarch64" ]; then
    echo "Error the system architecture is $ARCH_TYPE, is not in support lsit [x86_64, aarch64]"
    exit 1
fi

ARCH=""
if [ "$ARCH_TYPE" ==  "aarch64" ];then
    ARCH="aarch64-linux"
else
    ARCH="linux-64"
fi

# config install infomation
PACKAGE_VERSION=$VERSION
PACKAGE_NAME=Ascend-cann-${NAME}_${PACKAGE_VERSION}_linux-${ARCH_TYPE}.run
PACKAGE_PATH=$PREFIX/Ascend/$PACKAGE_NAME

# config install path
INSTALL_PATH=$PREFIX/Ascend
if [ ! -d $INSTALL_PATH ];then
    mkdir -p $INSTALL_PATH
fi

#install package
chmod +x $PACKAGE_PATH
CANN_INSTALL_FILE="/etc/Ascend/ascend_cann_install.info"
if  [ -f $CANN_INSTALL_FILE ];then
    mv $CANN_INSTALL_FILE ${CANN_INSTALL_FILE}.bak
fi

echo "Y" | $PACKAGE_PATH --install --install-path=$INSTALL_PATH