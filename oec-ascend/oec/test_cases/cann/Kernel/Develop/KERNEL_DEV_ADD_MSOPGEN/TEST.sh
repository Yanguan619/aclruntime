#!/bin/bash
output_path="$OEC_OUTPUT_PATH"
msopgen gen -i add_custom.json -c ai_core-Ascend910B3 -lan cpp -out "$output_path"
cd "$output_path"
bash build.sh