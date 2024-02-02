#!/bin/bash
echo "start to build glm2 workload"

declare -i ret_ok=0
declare -i ret_error=1

CURDIR=$(dirname $(readlink -f $0))

file_change()
{
    return $ret_ok
}

function main()
{
    rm -rf ${CURDIR}/output/*
    mkdir -p ${CURDIR}/output
    echo "build call args:$@"
    bash $CURDIR/patch.sh loadcode "$@" || { echo "warn run patch failed"; return 1; }
    cp -rf ${CURDIR}/patchcode ${CURDIR}//output/code
    cp -rf ${CURDIR}/scripts/* ${CURDIR}//output/
    cp ${CURDIR}/../../../common -r ${CURDIR}//output/

    mkdir -p ${CURDIR}/output/config
    cp ${CURDIR}/../common/*  -r ${CURDIR}//output/config/
    cp ${CURDIR}/config/config.sh -r ${CURDIR}//output/config/
    [ -d ${CURDIR}/doc ] && cp ${CURDIR}/doc -r ${CURDIR}/output/

    file_change "$2" || { echo "file change failed"; return 1; }
}

main "$@"
exit $?
