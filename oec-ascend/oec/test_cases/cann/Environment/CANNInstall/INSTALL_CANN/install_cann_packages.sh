echo  ===============================================
echo  ==    CANN PACKAGES INSTALL UNINSTALL TEST   ==
echo  ===============================================

cd $1
echo try to find Ascend-cann packages in $1
test_path="$(realpath $2)"
install_path="${test_path}/Ascend"
install_tmpdir="${install_path}"
mkdir -p "$install_path"
mkdir -p "$install_tmpdir"
export TMP_DIR="$install_tmpdir"
function install(){
    package=$1
    count=$(find . -type f -name "*$package*" | wc -l)
    if [ "$count" -ne 1 ]; then
        echo "ERROR: numer of $package is not equal to 1"
        exit 1
    fi
    echo  ===============================================
    echo INSTALL ./$package*
    echo ">>>>>>>>>>>> ASCEND_HOME_PATH <<<<<<<<<<<<<<<<<"
    env |grep ASCEND_HOME_PATH
    echo  ===============================================
    echo ./*$package* --install --install-path="$install_path" --quiet
    chmod +x *$package*
    ./*$package* --install --quiet --install-path="$install_path"
    rst=$?
    if [[ $rst != 0 ]]; then
        exit $rst
    fi
}
function uninstall(){
    package=$1
    count=$(find . -type f -name "*$package*" | wc -l)
    if [ "$count" -ne 1 ]; then
        echo "ERROR: numer of $package is not equal to 1"
        exit 1
    fi
    echo ./*$package* --uninstall --install-path="$install_path"
    chmod +x *$package*
    ./*$package* --uninstall --install-path="$install_path"
    rst=$?
    if [[ $rst != 0 ]]; then
        exit $rst
    fi
}

install cann-toolkit
rst=$?
if [[ $rst != 0 ]]; then
    exit $rst
fi
source ${install_path}/ascend-toolkit/set_env.sh
install cann-kernels
rst=$?
if [[ $rst != 0 ]]; then
    exit $rst
fi
install cann-nnal
rst=$?
if [[ $rst != 0 ]]; then
    exit $rst
fi
uninstall cann-nnal
rst=$?
if [[ $rst != 0 ]]; then
    exit $rst
fi
uninstall cann-kernels
rst=$?
if [[ $rst != 0 ]]; then
    exit $rst
fi
uninstall cann-toolkit
rst=$?
if [[ $rst != 0 ]]; then
    exit $rst
fi

code=0
if [[ -d Ascend/ascend-toolkit ]]; then
    code=1
fi
rm -rf Ascend
exit $code
