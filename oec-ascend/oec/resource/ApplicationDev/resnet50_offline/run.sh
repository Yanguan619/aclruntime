mkdir build
cd build

g++ ../resnet50.cpp -o resnet50 -lascendcl -ldl -lpthread -L${ASCEND_HOME_PATH}/lib64 -I${ASCEND_HOME_PATH}/include
if [[ $? != 0 ]]; then
    cd ..
    rm -rf build
    exit -1
fi
./resnet50 $1
rst=$?
echo rst=$rst
cd ..
rm -rf build
exit $rst