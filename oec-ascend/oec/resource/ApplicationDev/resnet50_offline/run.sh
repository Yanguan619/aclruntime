
data=$1
output=$2
npu=$3
mkdir -p "$output/tmp/resnet50/model"
g++ resnet50.cpp -o "$output/tmp/resnet50/resnet50" -lascendcl -ldl -lpthread -L${ASCEND_HOME_PATH}/lib64 -I${ASCEND_HOME_PATH}/include
if [[ $? != 0 ]]; then
    cd ..
    rm -rf build
    exit -1
fi

cd "$output/tmp/resnet50"
atc --model="$data/model/resnet50.onnx" --framework=5 --output="model/resnet50" --input_shape="actual_input_1:1,3,224,224" --soc_version=$npu
cp -r "$data/data" "$output/tmp/resnet50"

./resnet50 "$output/tmp/resnet50"
rst=$?
echo rst=$rst

exit $rst