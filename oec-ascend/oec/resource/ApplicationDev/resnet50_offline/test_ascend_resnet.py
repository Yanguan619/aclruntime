import oec
class ATCTest(oec.TestCase):
    def execute_command(self):
        self.execute_command_with_cmd(f'{self.get_cmd()} --soc_version={self.context.infomation.get("NPU")}')


ATCTest(
    group=("集成测试","离线推理"),
    name="OFFLINE_ATC_RESNET50",
    cmd=f'atc --model={oec.Context.data_path}/model/resnet50.onnx --framework=5 --output={oec.Context.data_path}/model/resnet50 --input_shape="actual_input_1:1,3,224,224"'
)

oec.TestCase(
    group=("集成测试","离线推理"),
    name="OFFLINE_ACL_RESNET50",
    cmd=f'bash run.sh {oec.Context.data_path}'
)

oec.TestCase(
    group=("集成测试","离线推理"),
    name="OFFLINE_ATC_RESNET50_CLEAN",
    cmd=f'rm -rf {oec.Context.data_path}/model/resnet50.om && rm -rf fusion_result.json'
)