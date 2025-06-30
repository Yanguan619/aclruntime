output_path="$1/acnnAllGatherMatmul"
src_path=$(pwd)
mkdir -p "$output_path" 
cd "${output_path}"
cmake "$src_path" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_SKIP_RPATH=TRUE
make

cd bin
./testcase
echo $?
exit $?