set -e
echo `date`
SIP_ENV="$ASCEND_HOME_PATH/../../nnal/asdsip/set_env.sh"

source ${SIP_ENV}
mkdir -p "$OEC_OUTPUT_PATH"

g++  example.cpp \
    -I${ASCEND_HOME_PATH}/include/aclnn \
    -I${ASCEND_HOME_PATH}/include \
    -L${ASCEND_HOME_PATH}/lib64/ -lascendcl -lopapi -lnnopbase \
    -I${ASDSIP_HOME_PATH}/include \
    -L${ASDSIP_HOME_PATH}/lib -lmki \
    -L${ASDSIP_HOME_PATH}/lib -lasdsip \
    -L${ASDSIP_HOME_PATH}/lib -lasdsip_core \
    -L${ASDSIP_HOME_PATH}/lib -lasdsip_host \
    -o $OEC_OUTPUT_PATH/example
cd "$OEC_OUTPUT_PATH"

# export ASCEND_SLOG_PRINT_TO_STDOUT=1
# export ASCEND_GLOBAL_LOG_LEVEL=0
# export ASDOPS_LOG_TO_STDOUT=1
# export ASDOPS_LOG_LEVEL=WARN
./example 
# > example_0905.log