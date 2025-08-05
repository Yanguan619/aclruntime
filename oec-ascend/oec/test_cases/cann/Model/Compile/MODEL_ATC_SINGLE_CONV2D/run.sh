jsonfile=$(realpath "conv2d.json")
mkdir -p "$OEC_OUTPUT_PATH"
cd "$OEC_OUTPUT_PATH"
atc --singleop="$jsonfile" --output=out --soc_version=Ascend910B3