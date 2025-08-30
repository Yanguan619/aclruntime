source $ASCEND_HOME_PATH/bin/setenv.bash

#!/bin/bash

# 执行命令并捕获所有输出
output=$(asys diagnose -r=cpu_detect -d 0 --output="$OEC_OUTPUT_PATH" 2>&1)
exit_code=$?

# 将输出打印到终端
echo "$output"

# 检查输出中是否包含[ERROR]
if echo "$output" | grep -q "\[ERROR\]"; then
    exit 1
fi

# 如果没有错误，返回原始命令的退出码
exit $exit_code