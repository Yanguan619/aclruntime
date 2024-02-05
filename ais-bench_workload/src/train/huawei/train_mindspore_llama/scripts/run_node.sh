#!/bin/bash
. $WORK_PATH/common/common.sh
. $WORK_PATH/common/log_util.sh
. $WORK_PATH/common/node_common.sh

declare -i ret_ok=0
declare -i ret_failed=1

SOC_VERSION=`python3 -c 'import acl;print(acl.get_soc_name())'`
if [[ "$SOC_VERSION" =~ "910B1" || "$SOC_VERSION" =~ "910B2" || "$SOC_VERSION" =~ "910B3" || "$SOC_VERSION" =~ "910B4" ]];then
    LLAMA_RUN_YAML_NAME="run_llama${LLAMA_MODEL_TYPE}_${LLAMA_MODEL_SCALE}_910b.yaml"
else
    LLAMA_RUN_YAML_NAME="run_llama${LLAMA_MODEL_TYPE}_${LLAMA_MODEL_SCALE}.yaml"
fi

function get_node_rank_id_range()
{
    RANK_ID_RANGE="[0,8]"
    # get server node id default is 0
    : "${NODE_ID:=0}"
    # get rank start index
    if [[ $DEVICE_NUM == 1  && $RANK_SIZE == 1 ]];then
        : "${SINGLE_CARD_INDEX:=0}"
        RANK_START=$SINGLE_CARD_INDEX
    else
        # get rank start index
        RANK_START=`expr ${NODE_ID} \* $DEVICE_NUM`
    fi
    RANK_ID_MAX=$[DEVICE_NUM+RANK_START]
    RANK_ID_RANGE="[$RANK_START,$RANK_ID_MAX]"
}

function node_init()
{
    export PYTHONPATH=$WORK_PATH:$PYTHONPATH

    if [ $1 == "check" ];then
        # install pyyaml
        if pip show pyyaml >/dev/null 2>&1;then
            logger_Info "pyyaml exist, won't be installed again"
        else
            pip_cmd="pip install pyyaml"
            $pip_cmd || { logger_Warn "pyyaml install failed:$?";return $ret_failed; }
        fi
        # install mindformers
        if pip show mindformers >/dev/null 2>&1;then
            logger_Info "mindformers exist, won't be installed again"
        else
            cd $WORK_PATH/code
            bash build.sh || { logger_Warn "mindformers install failed:$?";return $ret_failed; }
            cd $WORK_PATH
        fi
    fi
    # for eval env set
    [ $1 == "eval" ] && { export RANK_SIZE=1; export DEVICE_ID=0; : "${SINGLE_CARD_INDEX:=0}";export RANK_ID=$SINGLE_CARD_INDEX; unset RANK_TABLE_FILE; }
    get_node_rank_id_range
    [[ -z "$RESULT_PATH" ]] || { mkdir -p $RESULT_PATH; }
}

function node_check()
{
    rank_table_path=${WORK_PATH}/${RANK_TABLE_FILE}
    node_common_check "${PYTHON_COMMAND}" "${RANK_SIZE}" "$rank_table_path" || { logger_Warn "node common check failed" ; return $ret_failed; }

    # check_mindspore_run_ok_Ascend ${PYTHON_COMMAND} || { logger_Warn "mindspore running failed" ; return $ret_failed; }
    check_mindspore_run_ok ${PYTHON_COMMAND} || { logger_Warn "mindspore running failed" ; return $ret_failed; }
    logger_Debug "mindspore running successfully"

    if [ "$LLAMA_RUN_MODE" == "full" ] || [ "$LLAMA_RUN_MODE" == "train" ];then
        check_file_valid "${WORK_PATH}/code/${PRETRAIN_DATA_PATH}" || { logger_Warn "PRETRAIN_DATA_PATH:${PRETRAIN_DATA_PATH} not valid path" ; return 1; }
        logger_Debug "PRETRAIN_DATA_PATH is valid"
    fi

    if [ "$LLAMA_RUN_MODE" == "full" ] || [ "$LLAMA_RUN_MODE" == "finetune" ];then
        check_file_valid "${WORK_PATH}/code/${FINETUNE_DATA_PATH}" || { logger_Warn "FINETUNE_DATA_PATH:${FINETUNE_DATA_PATH} not valid path" ; return 1; }
        logger_Debug "FINETUNE_DATA_PATH is valid"
        check_file_valid "${WORK_PATH}/code/${EVAL_DATASET_PATH}" || { logger_Warn "EVAL_DATASET_PATH:${EVAL_DATASET_PATH} not valid path" ; return 1; }
        logger_Debug "EVAL_DATASET_PATH is valid"
    fi

}

function ckpt_merge()
{
    transform_ckpt_path=$WORK_PATH/code/mindformers/tools/transform_ckpt.py
    result_output_path=$WORK_PATH/result/output
    cd $WORK_PATH
    # ckpt merge
    $PYTHON_COMMAND $transform_ckpt_path \
        --src_ckpt_strategy $result_output_path/strategy/ \
        --src_ckpt_dir $result_output_path/checkpoint/ \
        --dst_ckpt_dir $result_output_path/target_ckpt/ \
        --prefix "llama${LLAMA_MODEL_TYPE}_${LLAMA_MODEL_SCALE}" || { logger_Warn "ckpt merge failed, rank id range: $RANK_ID_RANGE" ; return $ret_failed; }
    rm -rf $result_output_path/checkpoint/
}

function node_train()
{
    logger_Info "node_train running"
    export LLAMA_CUR_RUN_MODE=$1
    source $WORK_PATH/config/config.sh
    $PYTHON_COMMAND $WORK_PATH/pre_conf_yaml.py $1 # change yaml params
    run_script_path=$WORK_PATH/code/scripts/
    run_yaml_path=$WORK_PATH/code/configs/llama${LLAMA_MODEL_TYPE}/$LLAMA_RUN_YAML_NAME
    rank_table_path=${WORK_PATH}/$RANK_TABLE_FILE
    # train run
    cd $run_script_path
    cmd="bash run_distribute.sh $rank_table_path $run_yaml_path $RANK_ID_RANGE $1"
    [ "$NODEINFO_FILE" != "" ] && cmd="$cmd $RANK_SIZE"
    echo "$cmd"
    $cmd || { logger_Warn "node_run failed, rank id range: $RANK_ID_RANGE" ; return $ret_failed; }
    mv $WORK_PATH/code/output/ $WORK_PATH/result/ || { logger_Warn "move output failed!" ; return $ret_failed; }
    return $ret_ok
}

function eval_run()
{
    logger_Info "eval_run running"
    $PYTHON_COMMAND $WORK_PATH/pre_conf_yaml.py $1 # change yaml params
    run_yaml_path=$WORK_PATH/code/configs/llama${LLAMA_MODEL_TYPE}/$LLAMA_RUN_YAML_NAME
    eval_dataset_path=$WORK_PATH/code/$EVAL_DATASET_PATH
    load_checkpoint_path=$WORK_PATH/result/output/target_ckpt/rank_0/llama${LLAMA_MODEL_TYPE}_${LLAMA_MODEL_SCALE}0.ckpt
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
    elif [ "$EVAL_DATASET_TYPE" == "squad" ];then
        echo "eval not supported yet"
    else
        echo "invalid eval mode"
        rm -rf $load_checkpoint_path
        return $ret_failed
    fi
    rm -rf $load_checkpoint_path
    return $ret_ok
}

function node_eval()
{
    logger_Info "node_eval running"
    if [ "$LLAMA_RUN_MODE" == "full" ];then
        eval_run "finetune_eval"
    elif [ "$LLAMA_RUN_MODE" == "only_pretrain" ];then
        echo "eval not supported yet"
    elif [ "$LLAMA_RUN_MODE" == "only_finetune" ];then
        eval_run "finetune_eval"
    else
        echo "llama run mode not supported"
        return $ret_failed
    fi
    return $ret_ok
}

main()
{
    type="$1"
    mode="$2"
    shift
    node_init $type || { logger_Warn "init failed"; return $ret_failed; }
    if [ "$type" == "train" ];then
        node_train $mode || { logger_Warn "run_node_train failed"; return $ret_failed; }
    elif [ "$type" == "merge" ];then
        ckpt_merge || { logger_Warn "ckpt_merge failed"; return $ret_failed; }
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
