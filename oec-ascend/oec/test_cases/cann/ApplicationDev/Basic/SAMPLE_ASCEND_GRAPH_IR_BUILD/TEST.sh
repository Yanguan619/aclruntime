set -e
mkdir -p "$OEC_OUTPUT_PATH"
current_dir=$(pwd)/IRBuild
cd "$OEC_OUTPUT_PATH"
cmake "${current_dir}"
make
./ir_build Ascend310P3 "${OEC_DATA_PATH}/ir_build_data/"