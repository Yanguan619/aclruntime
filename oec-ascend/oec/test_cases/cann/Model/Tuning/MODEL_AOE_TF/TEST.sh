mkdir -p "$OEC_OUTPUT_PATH"
cd "$OEC_OUTPUT_PATH"
timeout -k 1s 1200s aoe --framework=3 --model="$OEC_DATA_PATH/model/model_tf.pb" --job_type=2