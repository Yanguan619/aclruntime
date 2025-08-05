#! /bin/bash
src_path=$(pwd)
argv=4
data_path="$OEC_DATA_PATH"
output_path="$OEC_OUTPUT_PATH"

mkdir -p "${output_path}"
cd "${output_path}"
cmake "${src_path}"
make
./main "${argv}" "${data_path}" "${output_path}"