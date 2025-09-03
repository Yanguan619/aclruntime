# oec-ascend

## 介绍

oec-ascend （昇腾软件操作系统兼容性验证工具）包含一套基础测试用例，用于检查操作系统软件和昇腾软件之间的兼容性。

## 功能

当前支持以下五个维度的功能验证

- 运行环境
- 应用开发
- 算子
- 模型开发
- 集成测试

## 快速开始
### 环境准备
请准备一台安装有昇腾NPU的环境，x86_64/ARM64,建议运行内存大小大于500GB

当前仅支持在 A2/A3/A300产品上运行，**其他产品上运行后的行为是未定义的**。

### 必要步骤

1. 访问 [昇腾社区资源中心](https://www.hiascend.com/developer/download/community) ，获取昇腾驱动固件和CANN软件安装包。
2. 请参考 [昇腾社区文档](https://www.hiascend.com/document)，安装昇腾驱动，固件软件。
3. 请参考 [昇腾社区文档](https://www.hiascend.com/document)，安装Ascend-cann-Toolkit，Ascend-cann-kernels，Ascend-cann-nnal等CANN软件。
4. 安装后检查您的安装流程是否有遗漏，请务必确保CANN软件和驱动软件运行**所需要的依赖已经正确安装**
#### 系统依赖
除了安装Ascend驱动固件，CANN软件包所必须安装的依赖外，您**还需要安装 cmake, g++** 用于支持部分测试用例在您的系统架构下自动构建。请参考您的操作系统的操作指南，安装这些第三方依赖

其中cmake建议版本大于 3.16，g++需要与gcc版本配套

#### 安装ais_bench_net_test工具
ais_bench_net_test是用于HCCL相关用例测试的python工具，如果您不需要测试hccl可以不安装，相关用例将直接失败跳过
请参考[ais_bench_net_test工具安装与卸载文档](https://gitee.com/ascend/tools/tree/develop/ais-bench_workload/tool/net_test#%E5%B7%A5%E5%85%B7%E5%AE%89%E8%A3%85%E4%B8%8E%E5%8D%B8%E8%BD%BD)

#### 准备测试资源
在**剩余空间充足**的硬盘上创建一个**空目录**，例如cann_test，
上传需要验证安装和卸载的toolkit,kernels,nnal的**run格式安装包**到该路径下,并给软件安装包**添加执行权限**，如果您不需要测试昇腾软件的安装卸载，可以不上传,相关用例将直接失败跳过
### 安装 oec-ascend工具
#### 通过预构建离线安装包安装

```
wget https://ascend-cann-open.obs.cn-north-4.myhuaweicloud.com/cann-os/oec_ascend_compatibility-1.0.0-py3-none-any.whl
pip install oec_ascend_compatibility-1.0.0-py3-none-any.whl
```
#### 运行

**注意**：请按照CANN软件安装指南中的说明配置当前环境安装的CANN的环境变量，如果不配置环境变量，默认使用usr/local/Ascend目录下的CANN软件包

```bash
oec-ascend --product A2/A3/A5/A300 --target all/cann
```
##### 参数说明
--product,-p    被测试的环境产品形态，当前支持A2/A3/A5/A300系列产品，其中A5为预留参数，当前A5测试范围和A2相同。

--target, -t    被测试环境中需要测试功能组件，当前支持 all/cann。all测试所有功能组件，cann仅测试CANN组件，当前这两个选项效果完全相同，后续会添加hdk等组件扩展工具能力。

### 查看运行报告
工具会在运行目录下生成"output/<时间戳>"目录存放测试报告，日志和临时文件，请下载 output/\<target>/<时间戳>/report.xlsx 文件到本地使用表格软件打开查看，时间戳为兼容性验证工具启动时的时间，可以在工具运行的打屏信息中找到报告的的生成路径。

其中：**sheet1为整体功能模块通过率,sheet2为各个测试用例运行情况和测试内容存放路径**

## 通过源码安装

```
git clone https://gitee.com/ascend/tools.git
cd tools/oec-ascend/oec
# 下载解压资源包
wget https://ascend-cann-open.obs.cn-north-4.myhuaweicloud.com/cann-os/data.tar
tar -xvf data.tar
#安装oec-ascend工具
cd ..
pip install -e .
```

后续使用方式与快速开始章节中使用章节内容一致

## 开发指导

在 oec/test_cases/\<target>/\<group>/\<class>/下添加你的测试用例目录

目录名称为测试用例名称，入口脚本为TEST.sh

框架会自动搜索和调用TEST.sh执行测试用例
