#!/bin/bash
echo `date`
NNAL_path="$ASCEND_HOME_PATH/../../nnal/atb/set_env.sh"
env_flag=0
source ${NNAL_path}

if [ ${env_flag} = 0 ]
then
    if test -e ./add_demo.cpp
    then
        mkdir -p "$OEC_OUTPUT_PATH"
        echo 'Compiling file with g++...'
        g++ -I "$ATB_HOME_PATH/include" -I "$ASCEND_HOME_PATH/include" -L "$ATB_HOME_PATH/lib" -L "$ASCEND_HOME_PATH/lib64" add_demo.cpp -latb -lascendcl -o "$OEC_OUTPUT_PATH/demo"
        cd "$OEC_OUTPUT_PATH"
        ./demo
        if [ $? = 0 ]
        then
            echo "Success!"
            exit 0
        fi
    fi
fi
exit 1