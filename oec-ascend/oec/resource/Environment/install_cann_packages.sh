echo  ===============================================
echo  ==    CANN PACKAGES INSTALL UNINSTALL TEST   ==
echo  ===============================================

cd $1
echo try to find Ascend-cann packages in $1
install_path=$(realpath $2)
mkdir -p "$install_path"

function install(){
    package=$1
    count=$(find . -type f -name "$package*" | wc -l)
    if [ "$count" -ne 1 ]; then
        echo "ERROR: numer of $package is not equal to 1"
        exit 1
    fi
    echo  ===============================================
    echo INSTALL ./$package*
    echo ">>>>>>>>>>>> ASCEND_HOME_PATH <<<<<<<<<<<<<<<<<"
    env |grep ASCEND_HOME_PATH
    echo  ===============================================
    echo ./$package* --install --install-path="$install_path" --quiet
    chmod +x $package*
    ./$package* --install --quiet --install-path="$install_path"
    if [[ $? != 0 ]]; then
        exit $?
    fi
}
function uninstall(){
    package=$1
    count=$(find . -type f -name "$package*" | wc -l)
    if [ "$count" -ne 1 ]; then
        echo "ERROR: numer of $package is not equal to 1"
        exit 1
    fi
    echo ./$package* --uninstall --install-path="$install_path"
    chmod +x $package*
    ./$package* --uninstall --install-path="$install_path"
    if [[ $? != 0 ]]; then
        exit $?
    fi
}

install Ascend-cann-toolkit
if [[ $? != 0 ]]; then
    exit $?
fi
source ${install_path}/ascend-toolkit/set_env.sh
install Ascend-cann-kernels
if [[ $? != 0 ]]; then
    exit $?
fi
install Ascend-cann-nnal
if [[ $? != 0 ]]; then
    exit $?
fi
uninstall Ascend-cann-nnal
if [[ $? != 0 ]]; then
    exit $?
fi
uninstall Ascend-cann-kernels
if [[ $? != 0 ]]; then
    exit $?
fi
uninstall Ascend-cann-toolkit
if [[ $? != 0 ]]; then
    exit $?
fi

code=0
if [[ -d Ascend/ascend-toolkit ]]; then
    code=1
fi
rm -rf Ascend
exit $code
