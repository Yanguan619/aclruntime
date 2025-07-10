import oec

oec.TestCase(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_ADD",
    cmd=f"bash runatc.sh '{oec.Context.output_dir}' add.json --output=out --soc_version=Ascend910B3",
    timeout=300
    )

oec.TestCase(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_CONV2D",
    cmd=f"bash runatc.sh '{oec.Context.output_dir}' conv2d.json --output=out --soc_version=Ascend910B3",
    timeout=300
    )

oec.TestCase(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_DYNAMIC_SHAPE",
    cmd=f"bash runatc.sh '{oec.Context.output_dir}' dynamic_shape.json --output=out --soc_version=Ascend910B3",
    timeout=300
    )

oec.TestCase(
    group= ("模型开发","模型编译"),
    name="MODEL_ATC_PB_TO_JSON_DESC",
    cmd=f"mkdir -p '{oec.Context.output_dir}/tmp' && cd '{oec.Context.output_dir}/tmp'\n"
        f"atc --mode=1 --om='{oec.Context.data_path}/model/model_tf.pb'  --json=out/model.json  --framework=3",
    timeout=300
)