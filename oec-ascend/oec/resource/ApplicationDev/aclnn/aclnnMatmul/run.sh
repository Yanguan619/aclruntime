mkdir -p build 
cd build
cmake ../ -DCMAKE_CXX_COMPILER=g++ -DCMAKE_SKIP_RPATH=TRUE
make

cd bin
./testcase
cd ../..
rm -rf build 
echo $?
exit $?