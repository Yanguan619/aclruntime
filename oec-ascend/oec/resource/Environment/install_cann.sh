cd $1
function install(){
    package=$1
    count=$(find . -type f -name "$package*" | wc -l)
    if [ "$count" -ne 1 ]; then
        echo numer of $package is not equal to 1
        exit 1
    fi
    echo ">>>>>>>>>>>> ASCEND_HOME_PATH <<<<<<<<<<<<<<<<<"
    env |grep ASCEND_HOME_PATH
    echo ">>>>>>>>>>>> INSTALL " ./$package* " <<<<<<<<<<<<<<<<<"
    echo ./$package* --install --quiet --install-path=$(realpath $1)/Ascend
    chmod +x $package*
    ./$2* --install --quiet --install-path=$(realpath $1)/Ascend
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
    echo ./$package* --uninstall --install-path=$(realpath $1)/Ascend
    chmod +x $package*
    ./$2* --uninstall --install-path=$(realpath $1)/Ascend
    if [[ $? != 0 ]]; then
        exit $?
    fi
}

install Ascend-cann-toolkit
source Ascend/ascend-toolkit/set_env.sh
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
