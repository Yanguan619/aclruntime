#!/bin/bash

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

# 配置训练相关的环境变量
source ${CODE_PATH}/config/config.sh || { logger_Warn "source file failed:$?";return $ret_init_failed; }

. $CODE_PATH/common/log_util.sh
. $CODE_PATH/common/common.sh

[ $LLAMA_RUN_MODE == "pretrain" ] && . $CODE_PATH/pretrain.sh
[ $LLAMA_RUN_MODE == "finetune" ] && . $CODE_PATH/finetune.sh

main(){
    pip_cmd="pip install pyyaml"
    $pip_cmd
    init || { logger_Warn "init failed:$?";return $ret_init_failed; }
    if [ $LLAMA_RUN_MODE == "pretrain" ];then
        run_train "pretrain" || { logger_Warn "run_train failed ret:$?";return $ret_run_train_failed; }
    elif [ $LLAMA_RUN_MODE == "finetune" ];then
        run_train "finetune" || { logger_Warn "run_train failed ret:$?";return $ret_run_train_failed; }
        run_eval "finetune" || { logger_Warn "run_eval failed ret:$?";return $ret_run_eval_failed; }
    else
        echo "wrong or empty llama run mode $LLAMA_RUN_MODE"
        return $ret_mode_failed
    fi
    get_result || { logger_Warn "get_result failed ret:$?";return $ret_get_result_failed; }
    return $ret_ok
}

main "$@"
exit $?
