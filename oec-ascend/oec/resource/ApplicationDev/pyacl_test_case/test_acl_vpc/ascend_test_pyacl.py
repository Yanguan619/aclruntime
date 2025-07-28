#encoding: utf-8
import oec

oec.TestCase(
    group= ("应用开发","媒体处理"),
    name = "PYACL_VPC",
    tags = [oec.app_dev, oec.pyacl, oec.media],
    cmd = f"python3 ./test_acl_vpc.py {oec.Context.data_path}"
    )
