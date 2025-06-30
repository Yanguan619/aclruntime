#!/bin/bash
output_path = $1
msopgen gen -i add_custom.json -c ai_core-Ascend910B3 -lan cpp -out "$output_path/AddKernelmsOpGen"
cd "$output_path/AddKernelmsOpGen"
bash build.sh