SOC_VERSION=$1

function make_run(){
    mkdir -p build
    cmake -B build \
        -DSOC_VERSION=${SOC_VERSION} \
        -DASCEND_CANN_PACKAGE_PATH=${ASCEND_HOME_PATH}
    if [ $? -ne 0 ]; then
        echo "cmake hello world failed"
        retrun 1
    fi
    cmake --build build -j
    if [ $? -e 0 ]; then
        echo "buid hello world failed"
        retrun 2
    fi
    cmake --install build
    if [ $? -ne 0 ]; then
        echo "install hello world failed"
        retrun 3
    fi
    check_msg="Hello World"
    file_path=output_msg.txt
    ./build/main
    count=$(grep -c "$check_msg" $file_path)
    if [ $count -ne 8 ]; then
        echo "Error, Expected 8 occurrences of $check_msg, but found $count occurrences."
        retrun 3
    fi

}


rm -rf build
rm -rf out
make_run
rm -rf build
rm -rf out