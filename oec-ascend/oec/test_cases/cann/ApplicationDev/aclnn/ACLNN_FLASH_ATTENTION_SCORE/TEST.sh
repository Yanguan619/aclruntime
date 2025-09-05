unsupported=(A300)
for product in "${unsupported[@]}"; do
    if [[ "$product" == "$OEC_PRODUCT" ]]; then
        exit 192
    fi
done
output_path="$OEC_OUTPUT_PATH"
src_path=$(pwd)
mkdir -p "$output_path" 
cd "${output_path}"
cmake "$src_path" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_SKIP_RPATH=TRUE
make

cd bin
./testcase
exit $?