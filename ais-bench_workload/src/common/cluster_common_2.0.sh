#!bin/bash

declare -i ret_ok=0
declare -i ret_failed=1

local_run_cmd()
{
    local cmd="$1"
    (eval "$cmd") || { echo "Warn local run '${cmd}'"; return $ret_failed; }
}

local_cp_cmd()
{
    local src_path="$1"
    local dst_path="$2"
    if [ -f $src_path ]; then
        cp -f $src_path $dst_path || { echo "cp: $src_path to $dst_path failed!";return $ret_failed; }
    elif [ -d $src_path ]; then
        cp -rf $src_path $dst_path || { echo "cp: $src_path to $dst_path failed!";return $ret_failed; }
    else
        echo "Warn src_path:$src_path not exist return"
        return $ret_failed
    fi
    return $ret_ok
}

local_put_cmd()
{
    local src_path="$1"
    local dst_path=$WORK_PATH/"$2"
    local_cp_cmd $src_path $dst_path || { return $ret_failed; }
    return $ret_ok
}

local_get_cmd()
{
    local src_path=$WORK_PATH/"$1"
    local dst_path="$2"
    local_cp_cmd $src_path $dst_path || { return $ret_failed; }
    return $ret_ok
}

cluster_init()
{
    if [ -n "$CLUSTER_SSH_KEY_PATH" ];then
        $PYTHON_COMMAND -m ais_bench.cluster init -n $NODEINFO_FILE -s $CLUSTER_SSH_KEY_PATH || { return $ret_failed; }
    elif [ $CLUSTER_AUTO_SET_KEY == 'on' ];then
        $PYTHON_COMMAND -m ais_bench.cluster init -n $NODEINFO_FILE -a  || { return $ret_failed; }
    else
        return $ret_failed
    fi
    return $ret_ok
}

cluster_multi_exec()
{
    [ "$NODEINFO_FILE" == "" ] && { local_run_cmd "$@";return $?; }
    local cmd="$1"
    local mode="$2"
    local device_num="$3"
    _cmd="$PYTHON_COMMAND -m ais_bench.cluster multi_exec -c '$cmd' "
    [ "$mode" != "" ] && _cmd="$_cmd -m $mode "
    [ "$device_num" != "" ] && _cmd="$_cmd -d $device_num "
    (eval "$_cmd") || { return $ret_failed; }
    return $ret_ok
}

cluster_single_exec()
{
    [ "$NODEINFO_FILE" == "" ] && { local_run_cmd "$@";return $?; }
    local cmd="$1"
    local node_id="$2"
    local device_num="$3"
    _cmd="$PYTHON_COMMAND -m ais_bench.cluster single_exec -c '$cmd' "
    [ "$node_id" != "" ] && _cmd="$_cmd -m $node_id "
    [ "$device_num" != "" ] && _cmd="$_cmd -d $device_num "
    (eval "$_cmd") || { return $ret_failed; }
    return $ret_ok
}

cluster_multi_put()
{
    [ "$NODEINFO_FILE" == "" ] && { local_put_cmd "$@";return $?; }
    local src="$1"
    local dst="$2"
    local mode="$3"
    _cmd="$PYTHON_COMMAND -m ais_bench.cluster multi_put -s $src -d $dst "
    [ "$mode" != "" ] && _cmd="$_cmd -m $mode "
    (eval "$_cmd") || { return $ret_failed; }
    return $ret_ok
}

cluster_single_put()
{
    [ "$NODEINFO_FILE" == "" ] && { local_put_cmd "$@";return $?; }
    local src="$1"
    local dst="$2"
    local mode="$3"
    _cmd="$PYTHON_COMMAND -m ais_bench.cluster single_put -s $src -d $dst "
    [ "$node_id" != "" ] && _cmd="$_cmd -m $node_id "
    (eval "$_cmd") || { return $ret_failed; }
    return $ret_ok
}

cluster_multi_get()
{
    [ "$NODEINFO_FILE" == "" ] && { local_get_cmd "$@";return $?; }
    local src="$1"
    local dst="$2"
    local mode="$3"
    _cmd="$PYTHON_COMMAND -m ais_bench.cluster multi_get -s $src -d $dst "
    [ "$mode" != "" ] && _cmd="$_cmd -m $mode "
    (eval "$_cmd") || { return $ret_failed; }
    return $ret_ok
}

cluster_single_get()
{
    [ "$NODEINFO_FILE" == "" ] && { local_get_cmd "$@";return $?; }
    local src="$1"
    local dst="$2"
    local mode="$3"
    _cmd="$PYTHON_COMMAND -m ais_bench.cluster single_get -s $src -d $dst "
    [ "$node_id" != "" ] && _cmd="$_cmd -m $node_id "
    (eval "$_cmd") || { return $ret_failed; }
    return $ret_ok
}