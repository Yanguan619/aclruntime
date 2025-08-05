# encoding: utf-8
from enum import Enum, unique


@unique
class State(Enum):
    NOT_RUNNING = "not running"
    RUNNING = "running"

    NOTHING_TO_DO = "nothing to do"
    PASS = "passed"
    WARNING = "warning"
    UNSUPPORTED = "unsupported"
    
    TIMEOUT = "timeout"
    FAIL = "failed"

# prodcuts

A2 = "A2"
A3 = "A3"
A5 = "A5"
A200 = "A200"
A300 = "A300"
A500 = "A500"
EXCLUDE_A200_A300_A500 = ["A2", "A3", "A5"]
EXCLUDE_A200_A500 = ["A2", "A3", "A5", "A300"]
ALL_PRODUCTS = ["A2", "A3", "A5", "A200", "A300", "A500"]

ALL_TARGETS = "cann hdk midie pta all".split()
# tags

acl = "acl"
pyacl = "pyacl"
cann = "cann"
combo_package = "combo_package"

env = "env"
env_os = "env_os"
env_drv = "env_drv"
env_pypi = "env_pypi"
env_cann = "env_cann"


app_dev = "app_dev"
media = "media"
aclnn = "aclnn"
atb = "atb"

kernel_dev = "kernel_dev"
bisheng = "bisheng"

model_dev = "model_dev"
hccl = "hccl"
atc = "atc"
aoe = "aoe"

