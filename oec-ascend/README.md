# oec-ascend

## 介绍

昇腾提供了OS兼容性验证工具（oec-ascend），包含一套基础测试用例，用于检查操作系统和昇腾软件之间的兼容性。

### 功能

当前支持以下五个维度的功能验证

- 运行环境
- 应用开发
- 算子
- 模型开发
- 集成测试
## 支持的产品
1. A2: Atlas A2训练系列产品，Atlas 800I A2推理产品
2. A3: Atlas A3训练系列产品，Atlas A3 推理系列产品
3. A300: 安装有 Atlas 300I Pro、Atlas 300V Pro、Atlas 300I Duo的推理服务器

## 安装前准备
1. 请准备一台安装有昇腾NPU的环境。**建议运行内存大于96GB，剩余硬盘空间大于100GB**
2. 请参考 [昇腾社区文档](https://www.hiascend.com/document)，安装昇腾驱动，固件软件。
3. 请参考 [昇腾社区文档](https://www.hiascend.com/document)，安装toolkit，kernels，nnal。
4. 安装 cmake, g++。其中cmake建议版本大于 3.16，g++需要与环境上的gcc版本配套。
### 安装ais_bench_net_test工具
ais_bench_net_test工具用于测试和验证HCCL相关功能
请参考[ais_bench_net_test工具安装与卸载文档](https://gitee.com/ascend/tools/tree/develop/ais-bench_workload/tool/net_test#%E5%B7%A5%E5%85%B7%E5%AE%89%E8%A3%85%E4%B8%8E%E5%8D%B8%E8%BD%BD)安装ais_bench_net_test

### 创建并上传CANN软件到测试目录
选取一个剩余空间充足的硬盘（建议剩余硬盘空间大于100GB）继续以下操作
```bash
mkdir -p cann_test
cd cann_test
```
上传toolkit,kernels,nnal的**run格式安装包**到cann_test路径下,并给软件安装包**添加执行权限**，请勿修改软件包名称。
## 安装运行 oec-ascend工具
### 通过whl包安装
**注意**：请按照CANN软件安装指南中的说明配置当前环境安装的CANN的环境变量，如果不配置环境变量，默认使用usr/local/Ascend目录下的CANN软件包
```bash
wget https://ascend-cann-open.obs.cn-north-4.myhuaweicloud.com/cann-os/oec_ascend_compatibility-1.0.0-py3-none-any.whl
pip3 install oec_ascend_compatibility-1.0.0-py3-none-any.whl
oec-ascend --product A2 --target cann
```
**参数说明**

>--product,-p    被测试的环境产品形态, 请根据您的产品形态输入对应的值，当前支持A2/A3/A5/A300，其中A5为预留参数，当前A5测试范围和A2相同。

>--target, -t    被测试环境中需要测试功能组件，当前支持 all/cann。all测试所有功能组件，cann仅测试CANN组件，当前这两个选项效果完全相同，后续会添加hdk等组件扩展工具能力。

## 查看运行报告
工具会在运行目录下生成"output/<时间戳>/\<target>/"目录存放测试报告，日志和临时文件，请下载该路径下的report.xlsx 文件到本地使用表格软件打开查看，时间戳为兼容性验证工具启动时的时间，可以在工具运行的打屏信息中找到报告的的生成路径。

其中：**sheet1为整体功能模块通过率,sheet2为各个测试用例运行情况和测试内容存放路径**

