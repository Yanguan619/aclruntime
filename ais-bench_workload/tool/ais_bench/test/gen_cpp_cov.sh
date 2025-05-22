#!/bin/bash
echo "************************Generate Coverage************************"

CUR_DIR=$(dirname $(readlink -f $0))
REL_DIR=$CUR_DIR/../output/release

if [ -d "./coverage" ]; then
    rm -rf ./coverage
fi
mkdir coverage

lcov_opt="--rc lcov_branch_coverage=1 --rc geninfo_no_exception_branch=1"
lcov -c -d ${REL_DIR}/backend/CMakeFiles/_backend_c.dir -o ./coverage/backend_test.info -b ./coverage $lcov_opt

lcov -r ./coverage/backend_test.info '*platform*' -o ./coverage/backend_test.info $lcov_opt
lcov -r ./coverage/backend_test.info '*opensource*' -o ./coverage/backend_test.info $lcov_opt
lcov -r ./coverage/backend_test.info '*test*' -o ./coverage/backend_test.info $lcov_opt
lcov -r ./coverage/backend_test.info '*c++*' -o ./coverage/backend_test.info $lcov_opt
lcov -r ./coverage/backend_test.info '*/usr/include/*' -o ./coverage/backend_test.info $lcov_opt
lcov -r ./coverage/backend_test.info '*nlohmann*' -o ./coverage/backend_test.info $lcov_opt
lcov -r ./coverage/backend_test.info '*anaconda*' -o ./coverage/backend_test.info $lcov_opt
lcov -r ./coverage/backend_test.info '*pybind11*' -o ./coverage/backend_test.info $lcov_opt

genhtml ./coverage/backend_test.info -o ./coverage/report --branch-coverage

cd coverage

tar -zcvf report.tar.gz ./report