set -e
bash build.sh
source $ASCEND_HOME_PATH/bin/setenv.bash
cd "$OEC_OUTPUT_PATH/tmp/resnet50"
./resnet50 "$OEC_OUTPUT_PATH/tmp/resnet50" 5000
asys collect --task_dir="$(pwd)" --tar="TRUE" --output=$OEC_OUTPUT_PATH/asys_output
