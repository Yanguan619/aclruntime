import oec

oec.TestCase(
    group=("集成测试","ATB"),
    name="ATB_MASH_UP_GRAPH",
    tags = [oec.app_dev, oec.atb],
    cmd=f'bash run.sh {oec.Context.cann_path} {oec.Context.output_dir}/tmp'
)