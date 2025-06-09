cd $1
echo $1 $2
count=$(find . -type f -name "$2*" | wc -l)
if [ "$count" -ne 1 ]; then
    echo numer of $2 is not equal to 1
    exit 1
fi
echo ">>>>>>>>>>>> ASCEND_HOME_PATH <<<<<<<<<<<<<<<<<"
env |grep ASCEND_HOME_PATH
echo ">>>>>>>>>>>> INSTALL " ./$2* " <<<<<<<<<<<<<<<<<"
echo ./$2* --install --quiet --install-path=$(realpath $1)/Ascend
chmod +x $2*
./$2* --install --quiet --install-path=$(realpath $1)/Ascend
exit $?
