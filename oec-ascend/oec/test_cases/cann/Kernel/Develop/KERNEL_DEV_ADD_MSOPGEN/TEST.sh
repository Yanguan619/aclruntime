#!/bin/bash
set -e
output_path="$OEC_OUTPUT_PATH"
special=(A300)
npu=$(python3 -c "
try:
    import acl
    print(acl.get_soc_name())
except:
    print('unknow')
")
target=ai_core-Ascend910B3
for product in "${special[@]}"; do
    if [[ "$product" == "$OEC_PRODUCT" ]]; then
        target="ai_core-$npu"
    fi
done
msopgen gen -i add_custom.json -c $target -lan cpp -out "$output_path"
cd "$output_path"
bash build.sh