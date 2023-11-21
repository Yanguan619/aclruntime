#!/bin/bash
echo "set env of llama train"
pretrained_origin_7b_ckpt_url=""
pretrained_origin_13b_ckpt_url=""
pretrained_converted_7b_ckpt_url="https://ascend-repo-modelzoo.obs.cn-east-2.myhuaweicloud.com/XFormer_for_mindspore/llama/open_llama_7b.ckpt"
pretrained_converted_13b_ckpt_url="https://ascend-repo-modelzoo.obs.cn-east-2.myhuaweicloud.com/XFormer_for_mindspore/llama/open_llama_13b.ckpt"
tokenizer_url="https://ascend-repo-modelzoo.obs.cn-east-2.myhuaweicloud.com/XFormer_for_mindspore/llama/tokenizer.model"
wikitest2_url="https://aisbenchtest.obs.myhuaweicloud.com/LLM_resource/llama/wikitext-2.tar.gz"
alpaca_url="https://aisbenchtest.obs.myhuaweicloud.com/LLM_resource/llama/alpaca_data.json"

export PYTHON_COMMAND=python3

export PRETRAIN_DATA_PATH=/home/datasets/imagenet/train/
export FINETUNE_DATA_PATH=/home/datasets/imagenet/train/
export EVAL_DATA_PATH=/home/datasets/imagenet/val/

export EPOCH_SIZE=90

export RANK_SIZE=8
export DEVICE_NUM=8

# need if rank_size > 1
export RANK_TABLE_FILE=/home/lcm/tool/rank_table_8p.json

# cluster need for node info
#export NODEINFO_FILE=/home/lcm/tool/ssh64_66.json
