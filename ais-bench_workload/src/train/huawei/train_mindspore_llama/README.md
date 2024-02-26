# 基于MindSpore/mindformers框架的llama大模型训练负载使用指南
本文主要介绍使用基于LLaMA 或LLaMA2 大模型训练业务代码构建的AISBench的负载包"train_huawei_train_mindspore_llama-Ais-Benchmark-Stubs-<arch>-2.0-r2.2.tar.gz"，进行服务器性能测试的流程。
## 名词定义
|名词|定义|
| --- | --- |
|管理节点|运行大模型训练负载的环境，只有一个|
|计算节点|执行训练任务的环境，可以有多个|
## 运行环境前置条件
### 管理节点
```
python >= 3.7
```
### 计算节点
```
mindspore >= 2.2
```
MindSpore安装参考[MindSpore官网](https://www.mindspore.cn/)MindSpore需要能成功在npu上运行，验证命令：
```bash
python -c "import mindspore;mindspore.set_context(device_target='Ascend');mindspore.run_check()"
```
如果正常输出：
```bash
MindSpore version: 版本号
The result of multiplication calculation is correct, MindSpore has been installed on platform [Ascend] successfully!
```
说明成功。
### 单机多卡与多机多卡的区别
单机多卡执行负载时，管理节点和唯一的计算节点是一个环境；多机多卡执行负载时，多机就是多个计算节点，管理节点必须是其中一个计算节点。<br>
**多机多卡需注意**
1. 为确保能操作计算节点的数据，管理节点需要是root用户
## 负载包中文件夹主要目录结构

```
├── ais-bench-stubs # Stubs主程序，负责流程控制、通信与数据管理等
├── code # 业务代码目录
│   ├── benchmark.sh # 入口脚本，会被ais-bench-stubs调用，调用业务代码，被测试者需要通过编写该脚本，对接运行的训练和推理脚本
│   ├── config
│   │   ├── config.sh # 训练相关的配置文件，包括数据集、权重路径等信息
│   └── code # mindformers全部代码，嵌入了AISBench的打点上报接口
│       └──mindformers
├── config
│   ├── config.json # 测试配置文件，包含tester服务器信息、testerId等信息
│   └── system.json # 被测试环境系统基本信息json文件，被测试者自行上传，比如硬件信息等
├── dependencies # stubs的依赖组件
│   ├── cluster # 分布式运行组件
│   │   ├── ais_bench_cluster-<version>-py3-none-linux_<arch>.whl
│   │   ├── README.md
│   └── logging # 测试结果传输模块
│       ├── ais_utils.py # 打点入口脚本，设置相关业务运行参数并反馈测试结果
│       └── libais_utils.so # 测试结果传输模块lib，负责将测试结果传输到stubs模块相关文件部署
├── log # 测试log日志。建议无需上传的日志文件，另建目录存放
├── result # 测试结果文件。建议无需上传的结果文件，另建目录存放
└── STUBS_PACKAGE_INTRO.md # Stubs被测试者接入使用文档
```
**后续对于相对路径的描述都是相对于负载包中的一级目录，例如 ./ais-bench-stubs表示Stubs主程序**
## 资源准备
### 前置声明
运行LLaMA（LLaMA2）训练的MindSpore/mindformers的代码全部在`./code/code`文件夹中，资源的准备参考[LLaMA资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/llama.md)或[LLaMA2资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/llama2.md)，具体资源的参考详见本章其他小节。<br>
**注意**：需要确认计算节点中是否原来已经安装了MindFormers，如果安装了，请使用`pip uninstall mindformers`卸载，确保负载代码的MindFormers能正常在计算节点中安装。
### rank_table_file准备(llama和llama2通用)
确保`/etc/hccn.conf`文件已经配好（如果没配好，参考[数据中心解决方案/配置训练节点](https://www.hiascend.com/document/detail/zh/Ascend%20Data%20Center%20Solution/22.0.0/install/800_9000/install_800_9000_0029.html)配置）。

参考[LLaMA资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/llama.md)的“生成RANK_TABLE_FILE(多卡运行必须环节)”和“多机RANK_TABLE_FILE合并(多机多卡必备环节)”章节。

### 模型权重下载与转换
LLaMA参考[LLaMA资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/llama.md)的“模型权重下载与转换”章节；
LLaMA2参考[LLaMA2资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/llama2.md)的“模型权重下载与转换”章节。

### 数据集准备
#### 预训练数据集准备
LLaMA参考[LLaMA资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/llama.md)的“预训练/数据集准备-预训练”章节；
LLaMA2参考[LLaMA2资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/llama2.md)“预训练/数据集准备”章节。
#### 微调数据集准备
LLaMA参考[LLaMA资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/llama.md)的“微调/数据集准备-微调”章节；
LLaMA2参考[LLaMA2资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/llama2.md)“微调/数据集准备”章节。
#### 评测数据集准备
**wikitext**
LLaMA参考[LLaMA资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/llama.md)的“评测/文本生成/获取数据集”章节；
LLaMA2参考[LLaMA2资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/llama2.md)“评测/文本生成/获取数据集”章节。

### nodeinfo_file准备（多机多卡训练需要）
nodeinfo_file为json文件，需要用户自行创建（如nodeinfo_file.json）并按照如下格式配置计算节点信息(不要把注释加进去)：
```json
{
    "0": { // 计算节点编号，为用户自定义，非设备实际编号，配置要求：不能重复、必须是0开始的连续整数，例如共有4个节点，节点编号只能取0，1，2，3。若不同节点配置了相同编号，那么只会读取其中一个节点的信息，另一个节点信息则被覆盖，实际运行测试时被覆盖的节点不会被测试。
        "ip": "xx.xx.xx.xx", //  计算节点的ip地址 ipv4
        "user": "user0", //  计算节点的用户名
        "port": 12345, // 访问计算节点的端口
        "work_path": "/xx/xx/xx/xx"  //  计算节点的工作路径，管理节点进入节点后处于的路径
    },
    "1":{
        ...
    }
    ...
}
```

## 负载启动前配置项
### 和tester连接的配置（仅在线测试需要）
`./config/config.json`和`./config/system.json`请参考《Stubs被测试者接入使用文档》中的“配置与Tester相关的配置文件”章节以及测试机构的要求进行配置。
### ./code/config/config.sh配置
`./code/config/config.sh`内容如下：
```bash
#!/bin/bash
echo "set env of llama train"
# 后续对于相对路径的描述都是相对于负载包中的一级目录，例如 ./ais-bench-stubs表示Stubs主程序

export PYTHON_COMMAND=python3
# 以下cluster配置二选一
export CLUSTER_SSH_KEY_PATH=~/.ssh/id_rsa # 用户指定的ssh私钥，需要确保管理节点通过此私钥能免密访问所有计算节点
export CLUSTER_AUTO_SET_KEY='on' # 'off' or 'on'，若为'on' 不需要配置CLUSTER_SSH_KEY_PATH


export LLAMA_MODEL_SCALE='7b' # '7b' 、'13b' 、 '70b'(仅llama2支持)
export LLAMA_MODEL_TYPE='' # llama : '' ; llama2 : '2'
export LLAMA_RUN_MODE='only_pretrain' # 'only_pretrain', 'only_finetune'，决定负载时执行预训练还是微调

# PRETRAIN_DATA_PATH, FINETUNE_DATA_PATH, EVAL_DATASET_PATH 这三个路径是相对./code/code/mindformers源码的路径, 必须以./mindformers/开头
export PRETRAIN_DATA_PATH=./mindformers/dataset_files/wikitext-2/wiki2048.mindrecord # 预训练数据集
export FINETUNE_DATA_PATH=./mindformers/dataset_files/alpaca-fastchat2048.mindrecord # 微调数据集
export EVAL_DATASET_TYPE='wikitext' # 评估数据集名，可选 'wikitext'
export EVAL_DATASET_PATH=./mindformers/dataset_files/wikitext-2/wiki2048valid.mindrecord # 评测用的数据集路径，必须以./mindformers/开头
export FINETUNE_CKPT_PATH=../../open_llama_7b.ckpt # only for 'only_finetune',相对于train_huawei_train_mindspore_llama-Ais-Benchmark-Stubs-<arch>-2.0-r2.2/code/目录的路径，权重放在code/内将自动分发到计算节点
export EVAL_DEVICE_ID=0 # 评测用的npu 的device id

export EPOCH_SIZE=1 # 全量遍历数据集的迭代次数
export LLAMA_LAYER_NUM=32 # 7b:32  13b:40  70b: 80

export RANK_SIZE=8 # 集群总加速卡数
export DEVICE_NUM=8 # 集群每个节点的加速卡数

# parallel run params, parallel strategy config, DATA_PARALLEL * MODEL_PARALLEL * PIPELINE_STAGE should equal to RANK_SIZE
export DATA_PARALLEL=2
export MODEL_PARALLEL=1
export PIPELINE_STAGE=4

# need if rank_size > 1
export RANK_TABLE_FILE=hccl_xxxx_8p.json # rank_table_file的路径，相对于train_huawei_train_mindspore_llama-Ais-Benchmark-Stubs-<arch>-2.0-r2.2/code/目录的路径，rank_table_file需要放在code/内

# 多机多卡需要配置，单机不能配置
#export NODEINFO_FILE=/home/lcm/tool/ssh64_66.json
```
请参考`./code/config/config.sh`的注释将第一章准备的资源的路径在`config.sh`中配置好，并且确定好训练相关的参数

## 负载启动
### 在线测试
在管理节点上执行命令
```bash
./ais-bench-stubs
```
### 轻量化离线测试
在管理节点上执行命令
```bash
./ais-bench-stubs test
```