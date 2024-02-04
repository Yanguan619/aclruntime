#!/bin/bash
echo "set env of llama train"
# 后续对于相对路径的描述都是相对于负载包中的一级目录，例如 ./ais-bench-stubs表示Stubs主程序

export PYTHON_COMMAND=python3
export CLUSTER_SSH_KEY_PATH=~/.ssh/id_rsa # the path of ssh private key path
export CLUSTER_AUTO_SET_KEY='on' # 'off' or 'on'

export LLAMA_MODEL_SCALE='7b' # '7b' 、'13b' 、 '70b'(仅llama2支持)
export LLAMA_MODEL_TYPE='' # llama : '' ; llama2 : '2'
export LLAMA_RUN_MODE='only_pretrain' # 'only_pretrain', 'only_finetune'，决定负载时执行预训练还是微调

# PRETRAIN_DATA_PATH, FINETUNE_DATA_PATH, EVAL_DATASET_PATH 这三个路径是相对./code/code/mindformers源码的路径, 必须以./mindformers/开头
export PRETRAIN_DATA_PATH=./mindformers/dataset_files/wikitext-2/wiki2048.mindrecord # 预训练数据集
export FINETUNE_DATA_PATH=./mindformers/dataset_files/alpaca-fastchat2048.mindrecord # 微调数据集
export EVAL_DATASET_TYPE='wikitext' # 评估数据集名，可选 'wikitext'
export EVAL_DATASET_PATH=./mindformers/dataset_files/wikitext-2/wiki2048valid.mindrecord # 评测用的数据集路径，必须以./mindformers/开头
export FINETUNE_CKPT_PATH=/home/data/ckpt/open_llama_7b.ckpt # only for 'only_finetune',相对于Ais-Benchmark-Stubs-<arch>/code/目录的路径，权重也要放在code/内
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
export RANK_TABLE_FILE=hccl_xxxx_8p.json # rank_table_file的路径，相对于Ais-Benchmark-Stubs-<arch>/code/目录的路径，rank_table_file需要放在code/内

# 多机多卡需要配置，单机不能配置
#export NODEINFO_FILE=/home/lcm/tool/ssh64_66.json
