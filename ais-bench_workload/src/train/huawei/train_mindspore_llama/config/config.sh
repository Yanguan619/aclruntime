#!/bin/bash
echo "set env of llama train"

export PYTHON_COMMAND=python3
export LLAMA_MODEL_TYPE='7b'
export LLAMA_RUN_MODE='only_pretrain' # 'only_pretrain', 'only_finetune', 'full'

export PRETRAIN_DATA_PATH=./dataset_files/wikitext-2/wiki2048.mindrecord # 相对mindformer源码的路径
export FINETUNE_DATA_PATH=./dataset_files/alpaca-fastchat2048.mindrecord # 相对mindformer源码的路径
export EVAL_DATASET_TYPE='wikitext' # 'wikitext' 'squad'
export EVAL_DATASET_PATH=./dataset_files/wikitext-2/wiki2048valid.mindrecord # 评测用的数据集路径
export EVAL_DEVICE_ID=0 # 评测用的npu

export EPOCH_SIZE=50
export LLAMA_LAYER_NUM=4 # 32

export RANK_SIZE=8
export DEVICE_NUM=8

# need if rank_size > 1
export RANK_TABLE_FILE=/home/hccl/hccl_xxxx_8p.json

# cluster need for node info
#export NODEINFO_FILE=/home/lcm/tool/ssh64_66.json
