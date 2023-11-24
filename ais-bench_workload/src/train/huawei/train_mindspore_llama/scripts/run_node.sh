#!/bin/bash
. $WORK_PATH/common/common.sh
. $WORK_PATH/common/log_util.sh
. $WORK_PATH/common/node_common.sh

declare -i ret_ok=0
declare -i ret_failed=1

RANK_ID_RANGE="[0,8]"

pretrained_converted_7b_ckpt_url="https://ascend-repo-modelzoo.obs.cn-east-2.myhuaweicloud.com/XFormer_for_mindspore/llama/open_llama_7b.ckpt"
pretrained_converted_13b_ckpt_url="https://ascend-repo-modelzoo.obs.cn-east-2.myhuaweicloud.com/XFormer_for_mindspore/llama/open_llama_13b.ckpt"
tokenizer_url="https://ascend-repo-modelzoo.obs.cn-east-2.myhuaweicloud.com/XFormer_for_mindspore/llama/tokenizer.model"
wikitest2_url="https://aisbenchtest.obs.myhuaweicloud.com/LLM_resource/llama/wikitext-2.tar.gz"
alpaca_url="https://aisbenchtest.obs.myhuaweicloud.com/LLM_resource/llama/alpaca_data.json"


function get_node_train_data()
{
    URL_DATA_PATH=$WORK_PATH/datas/
    if [ ! -d $URL_DATA_PATH ];then
        mkdir $URL_DATA_PATH
    fi
    # if [ ! -f $URL_DATA_PATH/tokenizer.model ];then
    #     wget -P $URL_DATA_PATH $tokenizer_url --no-check-certificate || { echo "wget $tokenizer_url failed!";return $ret_failed; }
    # fi
    # if [ ! -d $URL_DATA_PATH/wikitext-2/ ];then
    #     wget -P $URL_DATA_PATH $wikitest2_url --no-check-certificate || { echo "wget $wikitest2_url failed!";return $ret_failed; }
    #     tar xzf $URL_DATA_PATH/wikitext-2.tar.gz -C $URL_DATA_PATH
    #     rm -rf $URL_DATA_PATH/wikitext-2.tar.gz
    # fi
    # if [ ! -f $URL_DATA_PATH/alpaca_data.json ];then
    #     wget -P $URL_DATA_PATH $alpaca_url --no-check-certificate || { echo "wget $alpaca_url failed!";return $ret_failed; }
    # fi
    if [ "$LLAMA_RUN_MODE" = "only_finetune" ];then
        if [ "$LLAMA_MODEL_TYPE" = "7b" ] && [ ! -f $URL_DATA_PATH/open_llama_7b.ckpt ];then
            wget -P $URL_DATA_PATH $pretrained_converted_7b_ckpt_url --no-check-certificate || { echo "wget $pretrained_converted_7b_ckpt_url failed!";return $ret_failed; }
        fi
        if [ "$LLAMA_MODEL_TYPE" = "13b" ] && [ ! -f $URL_DATA_PATH/open_llama_13b.ckpt ];then
            wget -P $URL_DATA_PATH $pretrained_converted_13b_ckpt_url --no-check-certificate || { echo "wget $pretrained_converted_13b_ckpt_url failed!";return $ret_failed; }
        fi
    fi
    return $ret_ok
}

function get_node_rank_id_range()
{
    # get server node id default is 0
    : "${SERVER_ID:=0}"
    # get rank start index
    if [[ $DEVICE_NUM == 1  && $RANK_SIZE == 1 ]];then
        : "${SINGLE_CARD_INDEX:=0}"
        RANK_START=$SINGLE_CARD_INDEX
    else
        # get rank start index
        RANK_START=`expr ${SERVER_ID} \* $DEVICE_NUM`
    fi
    RANK_ID_MAX=$[DEVICE_NUM+RANK_START]
    RANK_ID_RANGE="[$RANK_START, $RANK_ID_MAX]"
}

function node_init()
{
    export PYTHONPATH=$PYTHONPATH:$WORK_PATH

    # install pyyaml
    if pip show pyyaml >/dev/null 2>&1;then
        pip_cmd="pip install pyyaml"
        $pip_cmd || { logger_Warn "pyyaml install failed:$?";return $ret_failed; }
    fi
    # install mindformers
    if pip show mindformers >/dev/null 2>&1;then
        cd $WORK_PATH/code
        bash build.sh || { logger_Warn "mindformers install failed:$?";return $ret_failed; }
        cd $WORK_PATH
    fi
    # for eval env set
    [ $1 == "eval" ] && { export RANK_SIZE=1; export DEVICE_ID=0; : "${SINGLE_CARD_INDEX:=0}";export RANK_ID=$SINGLE_CARD_INDEX; unset RANK_TABLE_FILE; }
    get_node_rank_id_range
    [[ -z "$RESULT_PATH" ]] || { mkdir -p $RESULT_PATH; }
}

function node_check()
{
    node_common_check "${PYTHON_COMMAND}" "${RANK_SIZE}" "$RANK_TABLE_FILE" || { logger_Warn "node common check failed" ; return $ret_failed; }

    check_mindspore_run_ok_Ascend ${PYTHON_COMMAND} || { logger_Warn "mindspore running failed" ; return $ret_failed; }
    logger_Debug "mindspore running successfully"

    if [ "$LLAMA_RUN_MODE" = "full" ] || [ "$LLAMA_RUN_MODE" = "pretrain" ];then
        check_path_valid "${PRETRAIN_DATA_PATH}" || { logger_Warn "TRAIN_DATA_PATH:${PRETRAIN_DATA_PATH} not valid path" ; return 1; }
        logger_Debug "PRETRAIN_DATA_PATH is valid"
    fi

    if [ "$LLAMA_RUN_MODE" = "full" ] || [ "$LLAMA_RUN_MODE" = "finetune" ];then
        check_path_valid "${FINETUNE_DATA_PATH}" || { logger_Warn "FINETUNE_DATA_PATH:${FINETUNE_DATA_PATH} not valid path" ; return 1; }
        logger_Debug "FINETUNE_DATA_PATH is valid"
        check_path_valid "${EVAL_DATASET_PATH}" || { logger_Warn "EVAL_DATASET_PATH:${EVAL_DATASET_PATH} not valid path" ; return 1; }
        logger_Debug "EVAL_DATASET_PATH is valid"
    fi

}

function node_run()
{
    $PYTHON_COMMAND $WORK_PATH/pre_conf_yaml.py $1 # change yaml params
    run_script_path=$WORK_PATH/code/scripts/run_distribute.sh
    run_yaml_path=$WORK_PATH/code/config/llama/$LLAMA_RUN_YAML_NAME
    result_output_path=$WORK_PATH/code/output
    transform_ckpt_path=$WORK_PATH/code/mindformers/tools/transform_ckpt.py
    # train run
    cmd="bash $run_script_path $RANK_FILE_PATH $run_yaml_path $RANK_ID_RANGE $1"
    $cmd || { logger_Warn "run finetune failed, , rank id range: $RANK_ID_RANGE" ; return $ret_failed; }
    # ckpt merge
    $PYTHON_COMMAND $transform_ckpt_path \
        --src_ckpt_strategy $result_output_path/strategy/ \
        --src_ckpt_dir $result_output_path/checkpoint/ \
        --dst_ckpt_dir $WORK_PATH/datas/target_ckpt/ \
        --prefix "llama_$LLAMA_MODEL_TYPE" || { logger_Warn "ckpt merge failed, rank id range: $RANK_ID_RANGE" ; return $ret_failed; }
    rm -rf $result_output_path/checkpoint/
    return $ret_ok
}

function node_train()
{
    if [ "$LLAMA_RUN_MODE" = "full" ];then
        node_run "pretrain" || { logger_Warn "run pretrain failed" ; return $ret_failed; }
        node_run "finetune" || { logger_Warn "run finetune failed" ; return $ret_failed; }
    elif [ "$LLAMA_RUN_MODE" = "only_pretrain" ];then
        node_run "pretrain" || { logger_Warn "run pretrain failed" ; return $ret_failed; }
    elif [ "$LLAMA_RUN_MODE" = "only_finetune" ];then
        node_run "finetune" || { logger_Warn "run finetune failed" ; return $ret_failed; }
    else
        echo "train run mode $LLAMA_RUN_MODE is invalid"
        return $ret_failed
    fi
    return $ret_ok
}

function eval_run()
{
    run_yaml_path=$WORK_PATH/code/config/llama/$LLAMA_RUN_YAML_NAME
    eval_dataset_path=$WORK_PATH/code/$EVAL_DATASET_PATH
    load_checkpoint_path=$WORK_PATH/datas/target_ckpt/llama_${LLAMA_MODEL_TYPE}0.ckpt
    if [ "$EVAL_DATASET_TYPE" = "wikitext" ];then
        echo "run eval using wiki"
        eval_script_path=$WORK_PATH/code/run_mindformer.py
        $PYTHON_COMMAND $eval_script_path \
            --config $run_yaml_path \
            --eval_dataset_dir $eval_dataset_path \
            --run_mode eval \
            --load_checkpoint $load_checkpoint_path \
            --epochs 1 \
            --use_parallel False \
            --device_id $EVAL_DEVICE_ID || { logger_Warn "run eval failed" ; return $ret_failed; }
    elif [ "$EVAL_DATASET_TYPE" = "squad" ];then
        echo "eval not supported yet"
    else
        echo "invalid eval mode"
        return $ret_failed
    fi
    return $ret_ok
}

function node_eval()
{
    if [ "$LLAMA_RUN_MODE" = "full" ];then
        eval_run
    elif [ "$LLAMA_RUN_MODE" = "only_pretrain" ];then
        echo "eval not supported yet"
    elif [ "$LLAMA_RUN_MODE" = "only_finetune" ];then
        eval_run
    else
        echo "llama run mode not supported"
        return $ret_failed
    fi
    return $ret_ok
}

main()
{
    type="$1"
    shift
    node_init "$@" || { logger_Warn "init failed"; return $ret_failed; }
    get_node_train_data
    if [ "$type" == "train" ];then
        node_train || { logger_Warn "run_node_train failed"; return $ret_failed; }
    elif [ "$type" == "eval" ];then
        node_eval || { logger_Warn "run_node_eval failed"; return $ret_failed; }
    elif [ "$type" == "check" ];then
        node_check || { logger_Warn "run_node_check failed"; return $ret_failed; }
    else
        { logger_Warn "invalid argument '${type}'"; return $ret_failed; }
    fi
}

main "$@"
exit $?
