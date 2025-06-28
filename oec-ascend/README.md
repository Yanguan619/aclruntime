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
#### 通过离线安装包安装
```
pip install oec_ascend_compatibility-1.0-py3-none-any.whl
```
#### 通过源码安装（需要开发或增加测试用例）
oec-ascend目录下执行
```
pip install -e .
```
### 使用
#### 环境要求
1、安装有昇腾NPU硬件并昇腾驱动软件
2、安装Ascend-Toolkit，Ascned-kernels，Ascend-nnal软件
3、环境上需要下载并解压oec-ascend OS兼容性检测工具所需的二进制数据包data.tar
4、将需要测试的Ascend-Toolkit，Ascned-kernels，Ascend-nnal放入解压后的二进制数据包根目录
#### 运行
选取一个空间足够的空目录
```bash
mkdir cann_test
cd cann_test
```
下载data.tar
```bash
tar -xvf data.tar
oec-ascend
```
#### 查看报告
下载 output/时间戳/report.xslx 文件到本地使用表格软件打开查看
#### 参数说明
--data 路径为环境要求第3,4点所下载的data.tar数据包解压后的目录
output 自行指定，为存放输出结果，日志和临时数据的输出路径，如果不指定，默认生成在当前目录的output目录下
