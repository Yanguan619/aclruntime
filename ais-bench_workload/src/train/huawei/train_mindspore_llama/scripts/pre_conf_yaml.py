import yaml
import os
import acl

cur_path = os.path.realpath(__file__)
config_path = os.path.join(cur_path, 'code/config/llama/')
try:
    soc_version = acl.get_soc_name()
except Exception as err:
    raise RuntimeError("get soc versiob failed!") from err
model_type = os.getenv('LLAMA_MODEL_TYPE')
run_mode = os.getenv("LLAMA_RUN_MODE")

if 'Ascend 910B'in soc_version:
    target_yaml = os.path.join(config_path, f'run_llama_{model_type}_910b.yaml')
else:
    target_yaml = os.path.join(config_path, f'run_llama_{model_type}.yaml')

if os.path.exists(target_yaml):
    raise RuntimeError(f"yaml file: {target_yaml} not find!")


def write_pretrain_yaml(data):
    pass


def write_finetune_yaml(data):
    pass


with open(target_yaml, 'r') as file:
    data = yaml.safe_load(file)

if run_mode == 'pretrain':
    write_pretrain_yaml(data)
elif run_mode == "finetune":
    write_finetune_yaml(data)
else:
    raise RuntimeError(f"run_mode{run_mode} not valid!")


