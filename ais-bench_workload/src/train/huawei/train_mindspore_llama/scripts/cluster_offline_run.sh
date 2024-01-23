#!/bin/bash
. $CODE_PATH/common/common.sh
. $CODE_PATH/common/log_util.sh
. $CODE_PATH/common/cluster_common_2.0.sh
. $CODE_PATH/common/node_common.sh

# env check
export RELAT_WORK_PATH=work
export RELAT_RESULT_PATH=$RELAT_WORK_PATH/result
CONFIG_FILE="config.sh"
local_env_cmd="source /etc/profile;
        export WORK_PATH=$WORK_PATH;
        export RESULT_PATH=$RESULT_PATH;
        export PYTHONPATH=$WORK_PATH:$PYTHONPATH;
        export PYTHONPATH=$WORK_PATH/logging:$PYTHONPATH;
        source $WORK_PATH/config/$CONFIG_FILE"
env_cmd="source /etc/profile;
        export WORK_PATH=\$PWD/$RELAT_WORK_PATH;
        export RESULT_PATH=\$PWD/$RELAT_RESULT_PATH;
        export PYTHONPATH=\$WORK_PATH:\$PYTHONPATH;
        export PYTHONPATH=\$WORK_PATH/logging:\$PYTHONPATH;
        source \$WORK_PATH/config/$CONFIG_FILE"

check_env()
{
    # check ranktable set
    : "${RANK_SIZE?RANK_SIZE not set}"
    : "${DEVICE_NUM?DEVICE_NUM not set}"
    [[ $RANK_SIZE -eq 1 ]] || : "${RANK_TABLE_FILE?RANK_TABLE_FILE not set}"
    [[ $RANK_SIZE -eq 1 ]] && [[ -n "$RANK_TABLE_FILE" ]] && { echo "ranksize=1 should not set RANK_TABLE_FILE";return 1; }

    # check python
    : "${PYTHON_COMMAND?PYTHON_COMMAND not set}"
    [ "$NODEINFO_FILE" == "" ] && { echo "NODEINFO_FILE not set, will not check cluster";return 0; }
    if pip show ais_bench_cluster >/dev/null 2>&1;then
        logger_Info "ais_bench cluster module exist, won't be installed again"
    else
        cluster_whl_path="${DEPEND_PATH}/cluster/ais_bench_cluster-*.whl"
        if [ -f $cluster_whl_path ];then
            pip install $cluster_whl_path --force-reinstall || { logger_Error "install cluster failed!";return 1; }
        else
            logger_Error "can't find ais_bench cluster wheel package"
        fi
    fi

    # check nodeinfofile exist
    [[ $RANK_SIZE -le 8 ]] || check_file_valid "${NODEINFO_FILE}" || { echo "nodeinfofile:${NODEINFO_FILE} not valid" ; return 1; }
}

init()
{
    logger_Info "-------------------------------- init start --------------------------------"
    # set nodes work path. 仅仅是管理节点的work/
    export WORK_PATH=${BASE_PATH}/work
    # set nodes result path
    export RESULT_PATH=${WORK_PATH}/result
    export PYTHONPATH=$PYTHONPATH:$CODE_PATH

    source ${CODE_PATH}/config/$CONFIG_FILE || { logger_Error "source file failed:$?";return 1; }
    if [ -d ${DEPEND_PATH}/logging ];then
        cp -r ${DEPEND_PATH}/logging ${CODE_PATH}
    fi
    check_env || { logger_Error "env check failed'" ; return 1; }

    # init ais_bench.cluster
    cluster_init || { logger_Error "ais_bench_cluster init failed!";return 1; }

    # refresh result path
    rm -rf $RESULT_PATH;mkdir -p $RESULT_PATH
    rm -rf $WORK_PATH;mkdir -p $WORK_PATH
    cp -r $CODE_PATH/* $WORK_PATH # CPU可以执行的都在host节点执行

    if [ "$NODEINFO_FILE" != "" ];then
         # sync data if work_path not exist so new one.节点的work/ 路径是相对于在node_file中指定的work_path
        cmd="rm -rf ${RELAT_WORK_PATH};mkdir -p ${RELAT_WORK_PATH}"
        cluster_multi_exec "$cmd" serial || { logger_Error "renew workpath failed"; return 1; }
         # copy code to node work path
        cluster_multi_put "$WORK_PATH" "./"  || { logger_Error "deploy code to work place failed"; return 1; }
    fi
    cmd="source /etc/profile;
       export WORK_PATH=\$PWD/$RELAT_WORK_PATH;
       source \$WORK_PATH/config/$CONFIG_FILE;
       bash \$WORK_PATH/run_node.sh check"
    cluster_multi_exec "$cmd" serial|| { return 1; }
    logger_Info "-------------------------------- init end --------------------------------"
}

run_train()
{
    logger_Info "-------------------------------- train start --------------------------------"
    if [ "$LLAMA_RUN_MODE" == "full" ] || [ "$LLAMA_RUN_MODE" == "only_pretrain" ];then
        if [ "$NODEINFO_FILE" == "" ];then
            cmd="$local_env_cmd;
            rm -rf $RESULT_PATH/*.json;
            bash $WORK_PATH/run_node.sh train train "
        else
            cmd="$env_cmd;
                rm -rf \$RESULT_PATH/*.json;
                bash \$WORK_PATH/run_node.sh train train "
        fi
        cluster_multi_exec "$cmd" || { logger_Error "run train(pretrain) failed"; return 1; }
        cluster_multi_get "$RELAT_RESULT_PATH" "$WORK_PATH" || { logger_Error "cp result between nodes failed"; return 1; }
        export PYTHONPATH=$WORK_PATH/logging:$PYTHONPATH
        bash $WORK_PATH/run_node.sh merge || { logger_Error "ckpt merge failed"; return 1; }
    fi
    if [ "$LLAMA_RUN_MODE" == "full" ] || [ "$LLAMA_RUN_MODE" == "only_finetune" ];then
        if [ "$NODEINFO_FILE" == "" ];then
            cmd="$local_env_cmd;
            rm -rf $RESULT_PATH/*.json;
            bash $WORK_PATH/run_node.sh train finetune "
        else
            cmd="$env_cmd;
                rm -rf \$RESULT_PATH/*.json;
                bash \$WORK_PATH/run_node.sh train finetune "
        fi
        cluster_multi_exec "$cmd" || { logger_Error "run train(finetune) failed"; return 1; }
        cluster_multi_get "$RELAT_RESULT_PATH" "$WORK_PATH" || { logger_Error "cp result between nodes failed"; return 1; }
        export PYTHONPATH=$WORK_PATH/logging:$PYTHONPATH
        bash $WORK_PATH/run_node.sh merge || { logger_Error "ckpt merge failed"; return 1; }
    fi
    logger_Info "-------------------------------- train end --------------------------------"
}

run_eval()
{
    logger_Info "-------------------------------- eval start --------------------------------"
    if [ "$NODEINFO_FILE" == "" ];then
        cmd="$local_env_cmd;
        bash $WORK_PATH/run_node.sh eval"
    else
        cmd="$env_cmd;
        bash \$WORK_PATH/run_node.sh eval"
    fi
    cluster_single_exec "$cmd" || { logger_Error "run eval failed"; return 1; }
    logger_Info "-------------------------------- eval end --------------------------------"
}

get_result()
{
    logger_Info "-------------------------------- get_result start --------------------------------"
    if [ "$NODEINFO_FILE" == "" ];then
        cmd="$local_env_cmd;
        mkdir -p $RESULT_PATH"
    else
        cmd="$env_cmd;
        mkdir -p \$RESULT_PATH"
    fi
    cluster_multi_exec "$cmd" serial || { logger_Error "mkdir resultpath failed"; return 1; }

    cluster_multi_get "${RELAT_RESULT_PATH}" "${WORK_PATH}" || { logger_Error "get result from ${RELAT_RESULT_PATH} failed"; return 1; }
    source ${CODE_PATH}/config/$CONFIG_FILE
    export PYTHONPATH=${CODE_PATH}/logging:$PYTHONPATH
    ${PYTHON_COMMAND} ${CODE_PATH}/common/calc_llm_result.py ${RESULT_PATH} ${RANK_SIZE} ${LLAMA_RUN_MODE}
    [ -d $BASE_PATH/result ] && cp ${RESULT_PATH}/* -rf  $BASE_PATH/result/
    find $BASE_PATH/result/ "*.ckpt" -type f -delete
    logger_Info "-------------------------------- get_result end --------------------------------"
}
