#! /bin/bash
src_path=$(pwd)
data_path="$OEC_DATA_PATH/data"
output_path="$OEC_OUTPUT_PATH"

mkdir -p "${output_path}"
cd "${output_path}"
cmake "${src_path}"
make
./main "${data_path}" "${output_path}/out_dir"