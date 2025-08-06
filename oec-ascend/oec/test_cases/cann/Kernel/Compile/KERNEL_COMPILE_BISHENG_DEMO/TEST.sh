RT_INC=${ASCEND_HOME_PATH}/runtime/include
RT_LIB=${ASCEND_HOME_PATH}/runtime/lib64
NPU=$(python3 -c "
try:
    import acl
    print(acl.get_soc_name())
except:
    print('unknow')
")
ouput="$OEC_OUTPUT_PATH"
mkdir -p "$ouput"
inputpath=$(pwd)
cd "$ouput"
# 功能：Host & Device代码混合编译，生成可执行文件,仅需链接libruntime.so
# 编译选项--cce-soc-version和--cce-soc-core-type指的是编译AscendXXXYY上的Vector核程序
bisheng -O2 --cce-soc-version=$NPU --cce-soc-core-type=VecCore  -I$RT_INC -L$RT_LIB -lascendcl -lruntime "$inputpath/QuickStartDemo.cce"  -o "QuickStartDemo"
rst=$?
if [[ $rst != 0 ]]; then
    exit $rst
fi
if [[ ! -f  QuickStartDemo  ]]; then
    exit 1
fi

