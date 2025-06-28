echo  ===============================================
echo  ==    CANN PACKAGES INSTALL UNINSTALL TEST   ==
echo  ===============================================

cd $1
install_path=$(realpath $2)
mkdir -p "$install_path"

function install(){
    package=$1
    count=$(find . -type f -name "$package*" | wc -l)
    if [ "$count" -ne 1 ]; then
        echo numer of $package is not equal to 1
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
        echo numer of $package is not equal to 1
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
source ${install_path}/ascend-toolkit/set_env.sh
install Ascend-cann-kernels
install Ascend-cann-nnal
uninstall Ascend-cann-nnal
uninstall Ascend-cann-kernels
uninstall Ascend-cann-toolkit

code=0
if [[ -d Ascend/ascend-toolkit ]]; then
    code=1
fi
rm -rf Ascend
exit $code
