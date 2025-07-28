import oec

oec.TestCase(
    group=("运行环境","运行依赖"),
    name='DEPENDENCY_DETECTION_OS',
    tags = [oec.env, oec.env_os],
    cmd='./dependency.sh',
    auxiliary=True
)

oec.TestCase(
    group=("运行环境","运行依赖"),
    name='DEPENDENCY_DETECTION_PYTHON',
    tags = [oec.env, oec.env_pypi],
    cmd='python3 check_package_version.py',
    auxiliary=True
)