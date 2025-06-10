from setuptools import setup, find_packages

setup(
    name="oec-ascend",
    version="0.4",
    packages=find_packages(),
    include_package_data=True,
    install_requires=["openpyxl", "pandas", "distro", "packaging"],
    author="spicy-bittern",
    author_email=" ",
    description="Ascend Operating System Compatibility Verification Tool",
    license="Apache-2.0",
    entry_points={  # 定义命令行指令
        "console_scripts": ["oec-ascend = oec.main:main"]  # 命令名 = 模块:函数
    },
    keywords="Ascend Operating System Compatibility Verification Tool oec-ascend",
    url="https://gitee.com/ascend/tools/tree/master/oec-ascend",
)
