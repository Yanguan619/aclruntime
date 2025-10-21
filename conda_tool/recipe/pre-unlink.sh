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
    ARCH="x86_64-linux"
fi

# config uninstall infomation
PACKAGE_VERSION=${VERSION}
PACKAGE_NAME=Ascend-cann-${NAME}_${PACKAGE_VERSION}_linux-${ARCH_TYPE}.run
PACKAGE_PATH=$PREFIX/Ascend/$PACKAGE_NAME

# uninstll package
UNINSTALL_FILE=$PREFIX/Ascend/${NAME}/${PACKAGE_VERSION}/${ARCH}/script/uninstall.sh
if [ "$NAME" == "toolkit" ];then
    UNINSTALL_FILE=$PREFIX/Ascend/ascend-${NAME}/${PACKAGE_VERSION}/${ARCH}/script/uninstall.sh

elif [ "$NAME" == "nnae" ];then
    UNINSTALL_FILE=$PREFIX/Ascend/${NAME}/${PACKAGE_VERSION}/script/uninstall.sh

elif [ "$NAME" == "nnal" ];then
    UNINSTALL_FILE=$PREFIX/Ascend/${NAME}/nnal_uninstall.sh
fi

if [[ "$NAME" =~ "kernels" ]];then
    UNINSTALL_FILE=$PREFIX/Ascend/ascend-toolkit/${PACKAGE_VERSION}/opp_kernel/script/uninstall.sh
fi

if [ -f $UNINSTALL_FILE ];then
    chmod +x $UNINSTALL_FILE
    echo "Y" | $UNINSTALL_FILE
else
    echo "$UNINSTALL_FILE is no existed, uninstall fail!"
fi