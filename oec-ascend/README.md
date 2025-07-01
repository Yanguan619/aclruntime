# oec-ascend

## 介绍

oec-ascend （昇腾软件操作系统兼容性验证工具）包含一套基础测试用例，用于检查操作系统软件和昇腾软件之间的兼容性。

## 功能

- 运行环境检查
- 应用开发检查
- 算子开发检查
- 模型开发检查
- 集成测试验证

## 快速开始
### 安装
#### 方式一：通过预构建离线安装包安装
```
wget https://ascend-cann-open.obs.cn-north-4.myhuaweicloud.com/cann-os/oec_ascend_compatibility-1.0-py3-none-any.whl
pip install oec_ascend_compatibility-1.0-py3-none-any.whl
```
#### 方式二：通过源码安装（需要开发或增加测试用例）
oec-ascend目录下执行
```
pip install -e .
```
### 使用
#### 必要步骤
昇腾驱动固件和CANN软件安装包，可以访问昇腾社区 https://www.hiascend.com/developer/download/community 获取

1、安装有昇腾NPU硬件并昇腾驱动软件

2、安装Ascend-cann-Toolkit，Ascend-cann-kernels，Ascend-cann-nnal软件
#### 推荐的第三方依赖包
系统依赖库
请使用您的操作系统的包管理器或者其它方式安装以下系统软件：

gcc,g++,cmake,make,tar,if-config(通常需要安装net-tools)

python第三方依赖：

您可以使用任何的python第三方库用于验证在您的OS上，昇腾软件和这些第三方库之间的兼容性，如果您不在意具体的软件包版本，可以安装以下我们推荐的第三方库

您可以在昇腾社区的CANN软件安装指南上找到CANN支持的第三方库库版本 [昇腾社区文档网站链接](https://www.hiascend.com/document)

```bash
# CANN依赖
pip install 'numpy>=1.19.2,<=1.24.0'
pip install 'decorator>=4.4.0'
pip install 'sympy>=1.5.1'
pip install 'cffi>=1.12.3'
pip install 'protobuf==3.20'
pip install attrs
pip install cython
pip install pyyaml
pip install pathlib2
pip install scipy
pip install requests
pip install psutil
pip install absl-py
```

#### 安装ais_bench_net_test工具
ais_bench_net_test是用于HCCL相关用例测试的python工具，如果您不需要测试hccl可以不安装，相关用例将直接失败跳过

请参考[ais_bench_net_test工具安装与卸载文档](https://gitee.com/ascend/tools/tree/develop/ais-bench_workload/tool/net_test#%E5%B7%A5%E5%85%B7%E5%AE%89%E8%A3%85%E4%B8%8E%E5%8D%B8%E8%BD%BD)
#### 准备测试资源
在剩余空间充足的硬盘上创建一个**空目录**，例如cann_test
下载data.tar，解压数据包
```bash
mkdir cann_test
cd cann_test
wget https://ascend-cann-open.obs.cn-north-4.myhuaweicloud.com/cann-os/data.tar
tar -xvf data.tar
```
#### 准备用于验证的CANN软件包
上传需要验证安装和卸载的toolkit,kernels,nnal的run格式安装包到data根路径下，如果您不需要测试昇腾软件的安装卸载，可以不上传,相关用例将直接失败跳过

#### 运行
在cann_test路径下，运行OS兼容性验证工具
```bash
oec-ascend
```
#### 查看报告
下载 output/时间戳/report.xslx 文件到本地使用表格软件打开查看
sheet1为整体功能模块通过率
sheet2为各个测试用例运行情况和测试内容存放路径