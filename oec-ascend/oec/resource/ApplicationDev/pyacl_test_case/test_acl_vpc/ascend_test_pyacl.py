#encoding: utf-8
import oec

oec.TestCase(
    group= ("应用开发","基础功能"),
    name = "PYACL_VPC",
    cmd = f"python3 ./test_acl_vpc.py {oec.Context.data_path}"
    )
