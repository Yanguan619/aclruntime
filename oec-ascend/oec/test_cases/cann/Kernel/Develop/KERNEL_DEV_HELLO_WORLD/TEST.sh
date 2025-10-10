SOC_VERSION=$(python3 -c "
try:
    import acl
    print(acl.get_soc_name())
except:
    print('unknow')
")
output="$OEC_OUTPUT_PATH"
type=dev
function make_run(){
    mkdir -p "$output/HelloWorld/build"
    
    cmake -B "$output/HelloWorld/build" \
        -DSOC_VERSION=${SOC_VERSION} \
        -DASCEND_CANN_PACKAGE_PATH=${ASCEND_HOME_PATH} \
        -DCMAKE_INSTALL_PREFIX="$output/HelloWorld/out"
    if [ $? -ne 0 ]; then
        echo "cmake hello world failed"
        return 1
    fi
    cd "$output/HelloWorld"
    cmake --build build -j
    if [ $? -ne 0 ]; then
        echo "buid hello world failed"
        return 2
    fi
    cmake --install build
    if [ $? -ne 0 ]; then
        echo "install hello world failed"
        return 3
    fi
    if [[ $type == "build" ]];then
        return 0 # 算子编译场景下无需执行用例，算子开发需要执行用例
    fi
    check_msg="Hello World"
    file_path=output_msg.txt

    ./build/main | tee $file_path
    count=$(grep -c "$check_msg" $file_path)

    if [ $count -ne 8 ]; then
        echo "Error, Expected 8 occurrences of $check_msg, but found $count occurrences."
        return 3
    fi

}


make_run
exit $?