#!/bin/bash
###
 # @Author: yanhe13 yanhe13@huawei.com
 # @Date: 2023-12-18 17:19:19
 # @LastEditors: yanhe13 yanhe13@huawei.com
 # @LastEditTime: 2024-01-16 10:16:26
 # @FilePath: \tools\ais-bench_workload\src\train\huawei\train_mindspore_llama\scripts\benchmark.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
###

# 遗留问题
# 1. mindformers的多机多卡的parallel_config怎么改？
# 2、yaml文件是否能够pretrain和finetune各一份？
# 3、如何判断llama跑完了，且没有出错？


# 返回码
declare -i ret_ok=0
declare -i ret_init_failed=1
declare -i ret_run_train_failed=2
declare -i ret_run_eval_failed=3
declare -i ret_get_result_failed=4
declare -i ret_mode_failed=5

CUR_PATH=$(dirname $(readlink -f "$0"))
export CODE_PATH=$CUR_PATH
export BASE_PATH=$(cd "$CUR_PATH/../";pwd)
export DEPEND_PATH=$BASE_PATH/dependencies/

function get_node_train_data()
{
    if [ "$LLAMA_RUN_MODE" = "only_finetune" ];then
        [ ! -f $FINETUNE_CKPT_PATH ] || { echo "finetune base ckpt:$FINETUNE_CKPT_PATH";return $ret_failed; }
    fi
    return $ret_ok
}

# 配置训练相关的环境变量
source ${CODE_PATH}/config/config.sh || { logger_Warn "source file failed:$?";return $ret_init_failed; }

. $CODE_PATH/common/log_util.sh
. $CODE_PATH/common/common.sh
if [ -d $PRETRAIN_DATA_PATH ];then
    cp -r $PRETRAIN_DATA_PATH $CUR_PATH || { logger_Warn "ERROR: cp $PRETRAIN_DATA_PATH failed!";return $ret_init_failed; }
fi
if [ -d $FINETUNE_DATA_PATH ];then
    cp -r $FINETUNE_DATA_PATH $CUR_PATH || { logger_Warn "ERROR: cp $FINETUNE_DATA_PATH failed!";return $ret_init_failed; }
fi

. $CODE_PATH/cluster_offline_run.sh

main(){
    get_node_train_data || { logger_Warn "download open llama cpkt failed:$?";return $ret_init_failed; }
    init || { logger_Warn "init failed:$?";return $ret_init_failed; }
    run_train || { logger_Warn "run_train failed ret:$?";return $ret_run_train_failed; }
    run_eval || { logger_Warn "run_eval failed ret:$?";return $ret_run_eval_failed; }
    get_result || { logger_Warn "get_result failed ret:$?";return $ret_get_result_failed; }
    return $ret_ok
}

main "$@"
exit $?
