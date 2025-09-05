
current_dir=$(pwd)
mkdir -p "$OEC_OUTPUT_PATH"

cd "$OEC_OUTPUT_PATH"
bishengir-compile "${current_dir}/demo.mlir" -o test
