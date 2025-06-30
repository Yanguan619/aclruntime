#encoding: utf-8
import oec

oec.TestCase(
    group= ("应用开发","基础功能"),
    name = "PYACL_OP",
    cmd = f"mkdir -p '{oec.Context.output_dir}/tmp/pyacl_testcase' && python3 ./test_acl_op.py {oec.Context.data_path} {oec.Context.output_dir}"
    )
