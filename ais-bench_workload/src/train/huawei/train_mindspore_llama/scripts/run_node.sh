#!/bin/bash
. $WORK_PATH/common/common.sh
. $WORK_PATH/common/log_util.sh
. $WORK_PATH/common/node_common.sh

declare -i ret_ok=0
declare -i ret_failed=1

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
    if [ ! -f $URL_DATA_PATH/tokenizer.model ];then
        wget -P $URL_DATA_PATH $tokenizer_url --no-check-certificate || { echo "wget $tokenizer_url failed!";return $ret_failed; }
    fi
    if [ ! -d $URL_DATA_PATH/wikitext-2/ ];then
        wget -P $URL_DATA_PATH $wikitest2_url --no-check-certificate || { echo "wget $wikitest2_url failed!";return $ret_failed; }
        tar xzf $URL_DATA_PATH/wikitext-2.tar.gz -C $URL_DATA_PATH
        rm -rf $URL_DATA_PATH/wikitext-2.tar.gz
    fi
    if [ ! -f $URL_DATA_PATH/alpaca_data.json ];then
        wget -P $URL_DATA_PATH $alpaca_url --no-check-certificate || { echo "wget $alpaca_url failed!";return $ret_failed; }
    fi
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
    return "[$RANK_START, $RANK_ID]"
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

    [[ -z "$RESULT_PATH" ]] || { mkdir -p $RESULT_PATH; }
}

function node_check()
{
    node_common_check "${PYTHON_COMMAND}" "${RANK_SIZE}" "$RANK_TABLE_FILE" || { logger_Warn "node common check failed" ; return $ret_failed; }

    check_mindspore_run_ok_Ascend ${PYTHON_COMMAND} || { logger_Warn "mindspore running failed" ; return $ret_failed; }
    logger_Debug "mindspore running successfully"
}

function node_pretrain()
{
    node_common_train "true" "false" || { logger_Warn "run train failed" ; return 1; }
}

function node_finetune()
{
    node_common_train "true" "false" || { logger_Warn "run train failed" ; return 1; }
}

function node_finetune_only()
{
    node_common_train "true" "false" || { logger_Warn "run train failed" ; return 1; }
}

function node_train()
{
    if [ "$LLAMA_RUN_MODE" = "full" ];then
        node_pretrain || { logger_Warn "run pretrain failed" ; return $ret_failed; }
        node_finetune || { logger_Warn "run finetune failed" ; return $ret_failed; }
    elif [ "$LLAMA_RUN_MODE" = "only_pretrain" ];then
        node_pretrain || { logger_Warn "run pretrain failed" ; return $ret_failed; }
    elif [ "$LLAMA_RUN_MODE" = "only_finetune" ];then
        node_finetune_only || { logger_Warn "run finetune failed" ; return $ret_failed; }
    else
        echo "train run mode $LLAMA_RUN_MODE is invalid"
        return $ret_failed
    fi
    return $ret_ok
}

function node_eval()
{
    if [ "$EVAL_DATASET_TYPE" = "wikitext" ];then
        echo "run eval using wiki"
    elif [ "$EVAL_DATASET_TYPE" = "squad" ];then
        echo "run eval using squad"
    else
        ehco "invalid eval mode"
    fi
}

main()
{
    type="$1"
    shift
    node_init "$@" || { logger_Warn "init failed"; return 1; }
    node_check
    get_node_train_data
    if [ "$type" == "train" ];then
        node_train || { logger_Warn "run_node_train failed"; return 1; }
    elif [ "$type" == "eval" ];then
        node_eval "$@" || { logger_Warn "run_node_eval failed"; return 1; }
    else
        { logger_Warn "invalid argument '${type}'"; return 1; }
    fi
}

main "$@"
exit $?
