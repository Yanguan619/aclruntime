#!/bin/bash
. $CODE_PATH/common/common.sh
. $CODE_PATH/common/log_util.sh
. $CODE_PATH/common/cluster_common.sh
. $CODE_PATH/common/node_common.sh

# env check
check_env()
{
    # mindspore check (also check cann)
    check_mindspore_run_ok_Ascend ${PYTHON_COMMAND} || { logger_Warn "check mindspore failed:$?";return 1; }

    # model info check
    : "${PRETRAIN_DATA_PATH?PRETRAIN_DATA_PATH not set}"

    # check env of each node
    cmd="export WORK_PATH=$WORK_PATH;
       bash $WORK_PATH/run_node.sh check ${WORK_PATH}/config/$CONFIG_FILE"
    cluster_run_cmd_serial "$NODEINFO_FILE" ${cmd} || { return 1; }
}

init()
{
    logger_Info "-------------------------------- init start --------------------------------"
    # set nodes work path
    export WORK_PATH=${BASE_PATH}/work
    # set nodes result path
    export RESULT_PATH=${WORK_PATH}/result
    export PYTHONPATH=$PYTHONPATH:$CODE_PATH

    CONFIG_FILE="config.sh"
    source ${CODE_PATH}/config/$CONFIG_FILE || { logger_Warn "source file failed:$?";return 1; }
    if [ -d ${BASE_PATH}/dependencies/logging ];then
        cp -r ${BASE_PATH}/dependencies/logging ${BASE_PATH}/code
    fi

    rm -rf $RESULT_PATH;mkdir -p $RESULT_PATH
    # sync data if work_path not exist so new one

    cmd="rm -rf ${WORK_PATH};mkdir -p ${WORK_PATH}"

    cluster_run_cmd_serial "$NODEINFO_FILE" ${cmd} || { logger_Warn "renew workpath failed"; return 1; }

    cluster_scp "${NODEINFO_FILE}" ${CODE_PATH} ${WORK_PATH}  || { logger_Warn "run scp failed"; return 1; }

    check_env || { logger_Warn "env check failed'" ; return 1; }
    logger_Info "-------------------------------- init end --------------------------------"
}

run_train()
{
    logger_Info "-------------------------------- train start --------------------------------"
    env_cmd="export WORK_PATH=$WORK_PATH;
        export RESULT_PATH=$RESULT_PATH;
        export PYTHONPATH=$WORK_PATH:$PYTHONPATH;
        export PYTHONPATH=$WORK_PATH/logging:$PYTHONPATH;
        source $WORK_PATH/config/$CONFIG_FILE"

    if [ "$LLAMA_RUN_MODE" == "full" ] || [ "$LLAMA_RUN_MODE" == "only_pretrain" ];then
        cmd="$env_cmd;
            rm -rf $RESULT_PATH/*.json;
            bash $WORK_PATH/run_node.sh train train"
        cluster_run_cmd_parallel "${NODEINFO_FILE}" ${cmd} || { logger_Warn "run train(pretrain) failed"; return 1; }
        if [ $NODEINFO_FILE != "" ];then
            cluster_rscp "${NODEINFO_FILE}" ${RESULT_PATH} ${RESULT_PATH} || { logger_Warn "cp result between nodes failed"; return 1; }
        fi
        $env_cmd
        bash $WORK_PATH/run_node.sh merge || { logger_Warn "ckpt merge failed"; return 1; }
    fi
    if [ "$LLAMA_RUN_MODE" == "full" ] || [ "$LLAMA_RUN_MODE" == "only_finetune" ];then
        cmd="$env_cmd;
            rm -rf $RESULT_PATH/*.json;
            bash $WORK_PATH/run_node.sh train finetune"
        cluster_run_cmd_parallel "${NODEINFO_FILE}" ${cmd} || { logger_Warn "run train(finetune) failed"; return 1; }
        if [ $NODEINFO_FILE != "" ];then
            cluster_rscp "${NODEINFO_FILE}" ${RESULT_PATH} ${RESULT_PATH} || { logger_Warn "cp result between nodes failed"; return 1; }
        fi
        $env_cmd
        bash $WORK_PATH/run_node.sh merge || { logger_Warn "ckpt merge failed"; return 1; }
    fi
    logger_Info "-------------------------------- train end --------------------------------"
}

run_eval()
{
    logger_Info "-------------------------------- eval start --------------------------------"
    cmd="source $WORK_PATH/config/$CONFIG_FILE;
        export WORK_PATH=$WORK_PATH;
        export RESULT_PATH=$RESULT_PATH;
        export PYTHONPATH=$WORK_PATH:$PYTHONPATH;
        export PYTHONPATH=$WORK_PATH/logging:$PYTHONPATH;
        bash $WORK_PATH/run_node.sh eval "
    cluster_run_cmd_single "${NODEINFO_FILE}" ${cmd} || { logger_Warn "run eval failed"; return 1; }
    logger_Info "-------------------------------- eval end --------------------------------"
}

get_result()
{
    logger_Info "-------------------------------- get_result start --------------------------------"
    cmd="mkdir -p ${RESULT_PATH};
        export PYTHONPATH=$WORK_PATH:$PYTHONPATH;
        export PYTHONPATH=$WORK_PATH/logging:$PYTHONPATH;"
    cluster_run_cmd_serial "$NODEINFO_FILE" ${cmd} || { logger_Warn "mkdir resultpath failed"; return 1; }

    cluster_rscp "${NODEINFO_FILE}" ${RESULT_PATH} ${RESULT_PATH}
    source ${CODE_PATH}/config/$CONFIG_FILE
    export PYTHONPATH=${CODE_PATH}/logging:$PYTHONPATH
    ${PYTHON_COMMAND} ${CODE_PATH}/common/calc_llm_result.py ${RESULT_PATH} ${RANK_SIZE} ${LLAMA_RUN_MODE}
    [ -d $BASE_PATH/result ] && cp ${RESULT_PATH}/* -rf  $BASE_PATH/result/
    logger_Info "-------------------------------- get_result end --------------------------------"
}
