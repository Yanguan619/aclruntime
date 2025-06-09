import oec

oec.TestCase(
    group= ("模型开发","模型调优"),
    name = "MODEL_AOE_TF",
    cmd=f"aoe --framework=3 --model={oec.Context.data_path}/model/model_tf.pb --job_type=2\n"
        "rm -rf aoe_workspace \n"
        "rm -rf kernel_meta \n"
        "rm -rf *.json"
    )