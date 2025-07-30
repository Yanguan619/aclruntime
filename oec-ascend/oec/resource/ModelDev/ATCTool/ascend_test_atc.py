import oec

oec.TestCase(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_ADD",
    products = ["A2", "A3", "A5", "A300"],
    tags = [oec.model_dev, oec.atc],
    cmd=f"bash runatc.sh '{oec.Context.output_dir}' add.json --output=out --soc_version=Ascend910B3",
    timeout=300
    )

oec.TestCase(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_CONV2D",
    products = oec.EXCLUDE_A200_A500,
    tags = [oec.model_dev, oec.atc],
    cmd=f"bash runatc.sh '{oec.Context.output_dir}' conv2d.json --output=out --soc_version=Ascend910B3",
    timeout=300
    )

oec.TestCase(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_DYNAMIC_SHAPE",
    products = oec.EXCLUDE_A200_A500,
    tags = [oec.model_dev, oec.atc],
    cmd=f"bash runatc.sh '{oec.Context.output_dir}' dynamic_shape.json --output=out --soc_version=Ascend910B3",
    timeout=300
    )

oec.TestCase(
    group= ("模型开发","模型编译"),
    name="MODEL_ATC_PB_TO_JSON_DESC",
    products = oec.EXCLUDE_A200_A500,
    tags = [oec.model_dev, oec.atc],
    cmd=f"mkdir -p '{oec.Context.output_dir}/tmp' && cd '{oec.Context.output_dir}/tmp'\n"
        f"atc --mode=1 --om='{oec.Context.data_path}/model/model_tf.pb'  --json=out/model.json  --framework=3",
    timeout=300
)