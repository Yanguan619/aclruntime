import oec
class ATCTest(oec.TestCase):
    def execute_command(self):
        self.execute_command_with_cmd(f'{self.get_cmd()} --soc_version={self.context.infomation.get("NPU")}')
ATCTest(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_ADD",
    cmd="atc --singleop=add.json --output=out/op_model1"
    )

ATCTest(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_CONV2D",
    cmd="atc --singleop=conv2d.json --output=out/op_model2",
    optional=True
    )

ATCTest(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_DYNAMIC_SHAPE",
    cmd="atc --singleop=dynamic_shape.json --output=out/op_model3",
    optional=True
    )

oec.TestCase(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_CLEAN_OUTPUT",
    cmd="rm -rf out && rm -rf fusion_result.json",
    auxiliary=True,
    optional=True
    )