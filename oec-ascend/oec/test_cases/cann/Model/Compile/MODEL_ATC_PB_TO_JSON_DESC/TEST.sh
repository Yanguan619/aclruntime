mkdir -p "$OEC_OUTPUT_PATH"
cd "$OEC_OUTPUT_PATH"
atc --mode=1 --om="${OEC_DATA_PATH}/model/model_tf.pb"  --json=model.json  --framework=3