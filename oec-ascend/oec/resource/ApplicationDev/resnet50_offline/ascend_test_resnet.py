import oec
class RESNET50Test(oec.TestCase):
    def execute_command(self):
        self.execute_command_with_cmd(
            f'bash run.sh {oec.Context.data_path} {oec.Context.output_dir} {self.context.infomation.get("NPU")}'
            )

RESNET50Test(
    group=("集成测试","离线推理"),
    name="OFFLINE_ACL_RESNET50",
)