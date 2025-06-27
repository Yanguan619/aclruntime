import oec

oec.TestCase(
    group=("集成测试","ATB"),
    name="ATB_MASH_UP_GRAPH",
    cmd=f'bash run.sh {oec.Context.cann_path} {oec.Context.output_dir}/tmp'
)