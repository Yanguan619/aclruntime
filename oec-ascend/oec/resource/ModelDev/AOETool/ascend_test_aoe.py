import oec

oec.TestCase(
    group= ("模型开发","模型调优"),
    name = "MODEL_ATC_SINGLE_ADD",
    cmd=f"aoe --framework=3 --model={oec.Context.data_path}/model/aoe_tf.pb --job_type=2 -o tf && rm -rf tf"
    )