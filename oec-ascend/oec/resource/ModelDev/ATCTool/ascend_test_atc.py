import oec
class ATCTest(oec.TestCase):
    def execute_command(self):
        import acl
        soc = acl.get_soc_name()
        self.execute_command_with_cmd(f"{self.get_cmd()} --soc_version={soc}")
ATCTest(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_ADD",
    cmd="atc --singleop=add.json --output=out/op_model1 && rm -rf out/op_model1"
    )

ATCTest(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_CONV2D",
    cmd="atc --singleop=conv2d.json --output=out/op_model2 && rm -rf out/op_model2"
    )

ATCTest(
    group= ("模型开发","模型编译"),
    name = "MODEL_ATC_SINGLE_DYNAMIC_SHAPE",
    cmd="atc --singleop=dynamic_shape.json --output=out/op_model3 && rm -rf out/op_model3"
    )