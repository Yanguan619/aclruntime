#!/bin/bash
echo "set env of glm train"

export PYTHON_COMMAND=python3
export CLUSTER_SSH_KEY_PATH=~/.ssh/id_rsa # the path of ssh private key path
export CLUSTER_AUTO_SET_KEY='on' # 'off' or 'on'

export GLM_RUN_MODE='only_finetune'

# PRETRAIN_DATA_PATH, FINETUNE_DATA_PATH, EVAL_DATASET_PATH 这三个路径是相对mindformers源码的路径, 必须以./mindformers/开头
export FINETUNE_DATA_PATH=./mindformers/dataset_files/AdvertiseGen/train.json # 微调数据集
export EVAL_DATASET_TYPE='ADGEN' # 'ADGEN'
export EVAL_DATASET_PATH=./mindformers/dataset_files/AdevertiseGen/dev.json # 评测用的数据集路径，必须以./mindformers/开头
export FINETUNE_CKPT_PATH=./mindformers/checkpoint_download/glm2/glm2_6b.ckpt # only for 'only_finetune'
export EVAL_DEVICE_ID=0 # 评测用的npu 的device id

export EPOCH_SIZE=1
export GLM_LAYER_NUM=4 # 7b:32  13b:40

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
