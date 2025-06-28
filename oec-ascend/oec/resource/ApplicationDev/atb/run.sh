cann_install_path=$1
output_path=$2
src_path=$(pwd)
source "${cann_install_path}/ascend-toolkit/set_env.sh"
source "${cann_install_path}/nnal/atb/set_env.sh"

function compile_model() {
    mkdir -p "${output_path}/atb_mash_up_graph"; 
    cd "${output_path}/atb_mash_up_graph";
    CXX11_ABI=$(env | awk -F'[=]' '/ATB_HOME_PATH/ {last=$2} END{print last}' | grep -oP 'cxx_abi_(\d)' | grep -oP '\d')
    CXX11_ABI=$(test "$CXX11_ABI" -eq 1 && echo "ON" || echo "OFF")
    echo "USE_CXX11_ABI=${CXX11_ABI}"
    cmake "${src_path}" -DUSE_CXX11_ABI="${CXX11_ABI}";
    if [ $? -ne 0 ]; then
        echo "ERROR: generate makefile failed!"
        exit 1
    fi

    cmake --build . -j;
    if [ $? -ne 0 ]; then
        echo "ERROR: compile test failed!"
        exit 1
    else
        echo "INFO: compile test succeed!"
    fi
    cd -;

}

compile_model
cd "${output_path}/atb_mash_up_graph"
./test_model
exit $?
