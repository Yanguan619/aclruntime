import os

ASCEND_HOME_PATH = os.environ.get("ASCEND_HOME_PATH")
if ASCEND_HOME_PATH is None:
    raise ValueError("ASCEND_HOME_PATH is not set")
realpath = os.path.realpath(f"{ASCEND_HOME_PATH}/runtime")
realpath = os.path.dirname(realpath)
version = os.path.basename(realpath)
if version is None:
    print(f"can not get cann version.ASCEND_HOME_PATH={ASCEND_HOME_PATH},cann path = {realpath}")
    exit(1)
print(version, end='')