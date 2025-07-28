import oec

oec.NPUTestCase(
    group=("集成测试","离线推理"),
    name="OFFLINE_ACL_RESNET50",
    tags = [oec.app_dev, oec.acl],
    cmd=f'bash run.sh {oec.Context.data_path} {oec.Context.output_dir} <NPU>'
)