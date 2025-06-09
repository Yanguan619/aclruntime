path=$(pwd)
cd $1
shift
mkdir -p tmp/atcout
cd tmp/atcout
atc $@

