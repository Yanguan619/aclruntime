#encoding: utf-8
import oec

oec.TestCase(
    group= ("应用开发","基础功能"),
    name = "PYACL_DEVICE",
    cmd = f"python3 ./test_acl_device.py",
    tags = [oec.app_dev, oec.pyacl],
    exclude=['failed',"ERROR","Error", "FAIL"],
    include="OK"
    )

oec.TestCase(
    group= ("应用开发","基础功能"),
    name = "PYACL_EVENT",
    tags = [oec.app_dev, oec.pyacl],
    cmd = f"python3 ./test_acl_event.py"
    )