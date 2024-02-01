import json
import os
import sys
import ais_utils

RESULT_PATH = sys.argv[1]
RANK_SIZE = sys.argv[2]
RUN_MODE = sys.argv[3]

total_throughput = 0
accuracy = 0
merged_json_data = {}
# get all json data
for rank_id in range(int(RANK_SIZE)):
    json_path = os.path.join(RESULT_PATH, f"result_rank_{rank_id}.json")
    if not os.path.exists(json_path):
        raise FileExistsError("{} file not exist".format(json_path))
    else:
        with open(json_path, "r") as file:
            json_data = json.load(file)
        if not merged_json_data:
            merged_json_data = json_data
        for mode_key, mode_value in json_data.items():
            for param_key, param_value in mode_value.items():
                merged_json_data[mode_key][param_key].extend(param_value)

# sort all json data
for mode_key, mode_value in merged_json_data.items():
    for param_key, param_value in mode_value.items():
        merged_json_data[mode_key][param_key].sort()

def set_result_single(mode:str):
    for key, value in merged_json_data[mode].items():
        if key == "throughput_ratio":
            ais_utils.set_result("training", key, sum(value))
        elif "start" in key:
            ais_utils.set_result("training", key, value[0])
        elif "end" in key:
            ais_utils.set_result("training", key, value[-1])


def set_result_full():
    pass


if RUN_MODE == "only_finetune":
    set_result_single("train")
else:
    raise RuntimeError(f"not supported run mode :{RUN_MODE}")
ais_utils.set_result("training", "result", "OK")