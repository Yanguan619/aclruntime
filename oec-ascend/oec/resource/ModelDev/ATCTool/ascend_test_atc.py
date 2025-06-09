import oec
class ATCTest(oec.TestCase):
    def execute_command(self):
        self.execute_command_with_cmd(f'{self.get_cmd()} --soc_version={self.context.infomation.get("NPU")}')
ATCTest(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_ADD",
    cmd=f"bash runatc.sh {oec.Context.output_dir} --singleop='$path/add.json' --output=out/op_model1"
    )

ATCTest(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_CONV2D",
    cmd=f"bash runatc.sh {oec.Context.output_dir} --singleop='$path/conv2d.json' --output=out/op_model2"
    )

ATCTest(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_DYNAMIC_SHAPE",
    cmd=f"bash runatc.sh {oec.Context.output_dir} --singleop='$path/dynamic_shape.json' --output=out/op_model3"
    )

oec.TestCase(
    group= ("模型开发","模型编译"),
    name="MODEL_ATC_PB_TO_JSON_DESC",
   cmd=f"bash runatc.sh {oec.Context.output_dir} --mode=1 --om={oec.Context.data_path}/model/model_tf.pb  --json=out/model.json  --framework=3"
)