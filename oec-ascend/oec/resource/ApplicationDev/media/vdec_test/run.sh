#! /bin/bash
src_path=$(pwd)
data_path=$1
output_path="$2/dvpp_vdec"

mkdir -p "${output_path}"
cd "${output_path}"
cmake "${src_path}"
make
./main "${data_path}" "${output_path}/out_dir"