import os

ASCEND_HOME_PATH = os.environ.get("ASCEND_HOME_PATH")

realpath = os.path.realpath(f"{ASCEND_HOME_PATH}/runtime")
realpath = os.path.dirname(realpath)

print(os.path.basename(realpath), end='')