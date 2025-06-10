jsonfile=$(realpath $2)
mkdir -p "$1/tmp" && cd "$1/tmp"
shift
shift
atc --singleop="$jsonfile" $@

