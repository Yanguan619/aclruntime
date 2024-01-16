#!/bin/bash
###
 # @Author: yanhe13 yanhe13@huawei.com
 # @Date: 2023-12-18 17:19:19
 # @LastEditors: yanhe13 yanhe13@huawei.com
 # @LastEditTime: 2024-01-16 10:34:13
 # @FilePath: \tools\ais-bench_workload\src\train\huawei\train_mindspore_llama\config\config.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
###
echo "set env of llama train"

export PYTHON_COMMAND=python3
export CLUSTER_SSH_KEY_PATH=~/.ssh/id_rsa
export CLUSTER_AUTO_SET_KEY='on' # 'off'

export LLAMA_MODEL_SCALE='7b' # '7b' or '13b'
export LLAMA_MODEL_TYPE='' # llama : '' ; llama2 : '2'
export LLAMA_RUN_MODE='only_pretrain' # 'only_pretrain', 'only_finetune', 'full'

# PRETRAIN_DATA_PATH, FINETUNE_DATA_PATH, EVAL_DATASET_PATH 这三个路径是相对mindformers源码的路径, 必须以./mindformers/开头
export PRETRAIN_DATA_PATH=./mindformers/dataset_files/wikitext-2/wiki2048.mindrecord # 预训练数据集
export FINETUNE_DATA_PATH=./mindformers/dataset_files/alpaca-fastchat2048.mindrecord # 微调数据集
export EVAL_DATASET_TYPE='wikitext' # 'wikitext' 'squad'
export EVAL_DATASET_PATH=./mindformers/dataset_files/wikitext-2/wiki2048valid.mindrecord # 评测用的数据集路径，必须以./mindformers/开头
export FINETUNE_CKPT_PATH=/home/data/ckpt/open_llama_7b.ckpt # only for 'only_finetune'
export EVAL_DEVICE_ID=0 # 评测用的npu 的device id

export EPOCH_SIZE=1
export LLAMA_LAYER_NUM=4 # 7b:32  13b:40

export RANK_SIZE=8 # 集群总加速卡数
export DEVICE_NUM=8 # 集群每个节点的加速卡数

# parallel run params, parallel strategy config, DATA_PARALLEL * MODEL_PARALLEL * PIPELINE_STAGE should equal to RANK_SIZE
export DATA_PARALLEL=2
export MODEL_PARALLEL=1
export PIPELINE_STAGE=4

# need if rank_size > 1
export RANK_TABLE_FILE=/home/hccl/hccl_xxxx_8p.json

# cluster need for node info
#export NODEINFO_FILE=/home/lcm/tool/ssh64_66.json
