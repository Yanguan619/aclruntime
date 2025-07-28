import oec

oec.TestCase(
    group= ("模型开发","模型调优"),
    name = "MODEL_AOE_TF",
    products = oec.EXCLUDE_A200_A500,
    tags = [oec.model_dev, oec.aoe],
    cmd=f"mkdir -p '{oec.Context.output_dir}/tmp'\n"
        f"cd '{oec.Context.output_dir}/tmp'\n"
        f"aoe --framework=3 --model={oec.Context.data_path}/model/model_tf.pb --job_type=2\n",
    timeout=900
    )