#! /bin/bash
src_path=$(pwd)
argv=$1
data_path=$2
output_path="$3/dvpp_vdec"

mkdir -p "${output_path}"
cd "${output_path}"
cmake "${src_path}"
make
./main "${argv}" "${data_path}" "${output_path}"