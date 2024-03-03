# 基于Mindspore/mindformers框架的glm2大模型训练负载使用指南
本文主要介绍使用基于glm2 大模型训练业务代码构建的AISBench的负载包，进行服务器性能测试的流程。
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
单机多卡执行负载，只在单机环境上部署和运行即可；多机多卡执行负载时，多机就是多个计算节点，管理节点必须是其中一个计算节点。<br>
**多机多卡需注意**
1. 如果管理节点不是计算节点，管理节点也需要安装与计算节点相同版本的mindspore，也需要安装训练负载包中的mindformers，mindformers的安装方式如下：
```bash
cd train_huawei_train_mindspore_llama-Ais-Benchmark-Stubs-<arch>-2.0-r2.2/code/code
pip install .
```
2. 为确保能操作计算节点的数据，管理节点需要是root用户
## 负载包中文件夹主要目录结构

```
├── ais-bench-stubs # Stubs主程序，负责流程控制、通信与数据管理等
├── code # 业务代码目录
│   ├── benchmark.sh # 入口脚本，会被ais-bench-stubs调用，调用业务代码，被测试者需要通过编写该脚本，对接运行的训练和推理脚本
│   ├── config
│   │   ├── config.sh # 训练相关的配置文件，包括数据集、权重路径等信息
│   ├── code # mindformers全部代码，嵌入了AISBench的打点上报接口
│   │   └──mindformers
│   ├── cluster_offline_run.sh
│   ├── run_node.sh
│   ├── run_glm2_6b_finetune.yaml
│   └── run_glm2_6b_finetune_eval.yaml
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
- **后续对于相对路径的描述都是相对于负载包中的一级目录，例如 ./ais-bench-stubs表示Stubs主程序**
- 管理节点安装mindformer需在code/code目录下执行`pip3 install .`
## 资源准备
### 前置声明
- 运行glm2训练的Mindspore/mindformers的代码全部在`./code/code`文件夹中，资源的准备参考[glm2资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/glm2.md),具体资源的参考详见本章其他小节。
- **注意**：需要确认环境中是否原来已经安装了mindformers，如果安装了，请使用`pip uninstall mindformers`卸载，确保负载代码的mindformers能正常安装。
### rank_table_file准备
- 确保`/etc/hccn.conf`文件已经配好（如果没配好，参考[数据中心解决方案/配置训练节点](https://www.hiascend.com/document/detail/zh/Ascend%20Data%20Center%20Solution/22.0.0/install/800_9000/install_800_9000_0029.html)配置）。

- 参考[glm2资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/glm2.md)的“生成RANK_TABLE_FILE”(单机多卡情况)和“多机RANK_TABLE_FILE合并”(多机多卡情况)章节。

### 模型权重下载与转换
- 参考[glm2资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/glm2.md)的“模型权重下载与转换”章节；
- 资源链接：
    - [glm2_6b.ckpt](https://ascend-repo-modelzoo.obs.cn-east-2.myhuaweicloud.com/XFormer_for_mindspore/glm2/glm2_6b.ckpt)(点击直接下载)
    - [tokenizer](https://ascend-repo-modelzoo.obs.cn-east-2.myhuaweicloud.com/XFormer_for_mindspore/glm2/tokenizer.model)(点击直接下载)
- 下载后建议放至code/code/mindformers/checkpoint_download/glm2目录下(需手动创建checkpoint_download/glm2 如`mkdir -p code/code/mindformers/checkpoint_download/glm2`)
### 数据集准备
- 参考[glm2资源准备](https://gitee.com/mindspore/mindformers/blob/ac5bb9ec8d1ea85fd2021ca5c6f13b6ae821c270/docs/model_cards/glm2.md)的“微调--数据集准备”章节；
- 资源链接：
    - [ADGEN数据集](https://cloud.tsinghua.edu.cn/f/b3f119a008264b1cabd1/?dl=1)(下载后需解压)
- 下载解压后目录结构为：
    ```
    AdvertiseGen
    ├── train.json
    └── dev.json
    ```
- 建议该目录放到code/code/mindformers/dataset_files/目录下(dataset_files需手动创建，如`mkdir -p code/code/mindformers/dataset_files/`)
### 1.4 nodeinfo_file准备（多机多卡训练需要）
nodeinfo_file为json文件，需要用户自行创建（如nodeinfo_file.json）并按照如下格式配置节点信息：
```json
{
    "0": { // 节点编号，为用户自定义，非设备实际编号，配置要求：不能重复、必须是0开始的连续整数，例如共有4个节点，节点编号只能取0，1，2，3。若不同节点配置了相同编号，那么只会读取其中一个节点的信息，另一个节点信息则被覆盖，实际运行测试时被覆盖的节点不会被测试。
        "ip": "xx.xx.xx.xx", // 节点的ip地址 ipv4
        "user": "user0", // 节点的用户名
        "port": 12345, // 访问节点的端口
        "work_path": "/xx/xx/xx/xx"  // 节点的工作路径，管理节点进入节点后处于的路径
    },
    "1":{
        ...
    }
    ...
}
```
**注意**：作为多机多卡时的管理节点的计算节点，work_path必须填写`train_huawei_train_mindspore_glm2-Ais-Benchmark-Stubs-{arch}-2.0-r2.2/`目录的绝对路径
## 2 负载启动前配置项
### 2.0 和tester连接的配置（仅在线测试需要）
`./config/config.json`和`./config/system.json`请参考《Stubs被测试者接入使用文档》中的“配置与Tester相关的配置文件”章节以及测试机构的要求进行配置。
### 2.1 ./code/config/config.sh配置
`./code/config/config.sh`内容如下：
```bash
#!/bin/bash
echo "set env of glm2 train"

export PYTHON_COMMAND=python3
#  以下cluster配置二选一，仅多机场景需要
export CLUSTER_SSH_KEY_PATH=~/.ssh/id_rsa # 用户指定的ssh私钥，确保通过此私钥管理节点能免密访问所有计算节点(单机场景注释此行)
export CLUSTER_AUTO_SET_KEY='on' # 'off' or 'on'， 若为'on' 不需要配置CLUSTER_SSH_KEY_PATH(单机场景注释此行)

export GLM_RUN_MODE='only_finetune'

# FINETUNE_CKPT_PATH, FINETUNE_DATA_PATH, EVAL_DATASET_PATH 这三个路径是相对mindformers源码的路径, 必须以./mindformers/开头
# 可以在下载对应数据集完成后，将其复制到code/code/mindformers目录下，比如新建一个dataset_files存放解压后的AdvertiseGen
export FINETUNE_DATA_PATH=./mindformers/dataset_files/AdvertiseGen/train.json # 微调数据集实际路径
export EVAL_DATASET_TYPE='ADGEN' # 'ADGEN'
export EVAL_DATASET_PATH=./mindformers/dataset_files/AdvertiseGen/dev.json # 评测用的数据集路径，必须以./mindformers/开头
export FINETUNE_CKPT_PATH=./mindformers/checkpoint_download/glm2/glm2_6b.ckpt # 微调使用的预训练权重，必须以./mindformers/开头
export EVAL_DEVICE_ID=0 # 评测用的npu 的device id

export EPOCH_SIZE=1
export GLM_LAYER_NUM=4

export RANK_SIZE=8 # 集群总加速卡数
export DEVICE_NUM=8 # 集群每个节点的加速卡数

# parallel run params, parallel strategy config, DATA_PARALLEL * MODEL_PARALLEL * PIPELINE_STAGE should equal to RANK_SIZE
export DATA_PARALLEL=2
export MODEL_PARALLEL=1
export PIPELINE_STAGE=4

# need if rank_size > 1
export RANK_TABLE_FILE=./hccl_xxxx_8p.json # 配置为生成的rank table路径，是相对于负载仓的code目录的路径，如果不在code目录下需要拷贝到code目录下

# 多机多卡需要配置，单机不需要配置
#export NODEINFO_FILE=/home/lcm/tool/ssh64_66.json
```

- 请参考`./code/config/config.sh`的注释将"资源准备"章节准备的资源的路径在`config.sh`中配置好，

### 2.2 yaml配置
- 前置声明：所有修改路径均为绝对路径
- 需要修改code/run_glm2_6b_finetune_eval.yaml:
```
train_dataset: &train_dataset
  data_loader:
    type: ADGenDataLoader
    dataset_dir: "/path/to/AdvertiseGen/train.json" # 需要修改为实际AdvertiseGen/train.json路径
    shuffle: True
    phase: "train"
    version: 2
    origin_columns: ["content", "summary"]
  tokenizer:
    type: ChatGLM2Tokenizer
    vocab_file: "/path/to/tokenizer.model" # 需要修改为实际tokenizer.model路径
  input_columns: ["input_ids", "labels"]
  max_source_length: 64
  max_target_length: 128
  ignore_pad_token_for_loss: True
  num_parallel_workers: 8
  python_multiprocessing: False
  drop_remainder: True
  batch_size: 1
  repeat: 1
  numa_enable: False
  prefetch_size: 1
  seed: 0

train_dataset_task:
  type: KeyWordGenDataset
  dataset_config: *train_dataset

eval_dataset: &eval_dataset
  data_loader:
    type: ADGenDataLoader
    dataset_dir: "/path/to/AdvertiseGen/dev.json" # 需要修改为实际AdvertiseGen/dev.json路径
    shuffle: False
    phase: "eval"
    version: 2
    origin_columns: ["content", "summary"]
  tokenizer:
    type: ChatGLM2Tokenizer
    vocab_file: "/path/to/tokenizer.model" # 需要修改为实际tokenizer.model路径
  max_source_length: 256
  max_target_length: 256
  ignore_pad_token_for_loss: True
  input_columns: ["input_ids", "labels"]
  num_parallel_workers: 8
  python_multiprocessing: False
  drop_remainder: True
  batch_size: 1
  repeat: 1
  numa_enable: False
  prefetch_size: 1
  seed: 0

eval_dataset_task:
  type: KeyWordGenDataset
  dataset_config: *eval_dataset
```

- 修改code/run_glm2_6b_finetune.yaml**同样需要修改上述的部分**，另外把文件开头的load_checkpoint设置为glm2的实际ckpt路径：
```
seed: 0
run_mode: 'train'
output_dir: './output'  # 当前不支持自定义修改，请勿修改该默认值
load_checkpoint: 'glm2_6b.ckpt' # 修改为实际下载的glm2_6b.ckpt路径
auto_trans_ckpt: False  # If true, auto transform load_checkpoint to load in distributed model
only_save_strategy: False
resume_training: False
```
## 3 负载启动
### 3.1 在线测试
执行命令
```bash
./ais-bench-stubs
```
### 3.2 轻量化离线测试
执行命令
```bash
./ais-bench-stubs test
```