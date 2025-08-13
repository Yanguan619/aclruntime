bash build.sh
source $ASCEND_HOME_PATH/bin/setenv.bash
cd "$OEC_OUTPUT_PATH/tmp/resnet50"

asys launch --task="./resnet50 \"$OEC_OUTPUT_PATH/tmp/resnet50\" 5000" --tar="TRUE" --output=$OEC_OUTPUT_PATH/asys_output

cd "$ASCEND_HOME_PATH/tools/msaicerr"
python3 msaicerr.py -p "$OEC_OUTPUT_PATH/asys_output" -out "$OEC_OUTPUT_PATH/result"
