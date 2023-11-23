
# should be execute after distribut
import sys
import os
import yaml
import acl

run_mode = sys.argv[1]
cur_path = os.path.realpath(__file__)
config_path = os.path.join(cur_path, 'code/config/llama/')
try:
    soc_version = acl.get_soc_name()
except Exception as err:
    raise RuntimeError("get soc versiob failed!") from err

pretrain_dataset = os.getenv('PRETRAIN_DATA_PATH')
finetune_dataset = os.getenv('FINETUNE_DATA_PATH')
model_type = os.getenv('LLAMA_MODEL_TYPE')
epoch_size = os.getenv("EPOCH_SIZE")
layer_num = os.getenv("LLAMA_LAYER_NUM")
eval_data_path = os.getenv('EVAL_DATASET_PATH')

if 'Ascend 910B'in soc_version:
    target_yaml = os.path.join(config_path, f'run_llama_{model_type}_910b.yaml')
else:
    target_yaml = os.path.join(config_path, f'run_llama_{model_type}.yaml')
if os.getenv('LLAMA_RUN_MODE') == 'only_finetune':
    ckpt_path = os.path.join(cur_path, f'../datas/open_llama_{model_type}')
else:
    ckpt_path = os.path.join(cur_path, f'../datas/target_ckpt/llama_{model_type}0.ckpt')

if not os.path.exists(target_yaml):
    raise RuntimeError(f"yaml file: {target_yaml} not find!")


def write_pretrain_yaml(data):
    data['load_checkpoint'] = ''
    data['run_mode'] = 'train'
    data['runner_config']['epochs'] = epoch_size
    data['optimizer']['beta2'] = 0.95
    data['optimizer']['learning_rate'] = 3.e-4
    data['lr_schedule']['learning_rate'] = 3.e-4
    data['lr_schedule']['lr_end'] = 3.e-5
    data['train_dataset']['input_columns'] = ["input_ids"]
    data['train_dataset']['data_loader']['dataset_dir'] = pretrain_dataset
    data['eval_dataset']['data_loader']['dataset_dir'] = eval_data_path
    data['model']['model_config']['num_layers'] = layer_num


def write_finetune_yaml(data):
    data['load_checkpoint'] = ckpt_path
    data['run_mode'] = 'finetune'
    data['runner_config']['epochs'] = epoch_size
    data['optimizer']['beta2'] = 0.999
    data['optimizer']['learning_rate'] = 1.e-5
    data['lr_schedule']['learning_rate'] = 1.e-5
    data['lr_schedule']['lr_end'] = 1.e-5
    data['train_dataset']['input_columns'] = ["input_ids", "labels"]
    data['train_dataset']['data_loader']['dataset_dir'] = finetune_dataset
    data['eval_dataset']['data_loader']['dataset_dir'] = eval_data_path
    data['model']['model_config']['num_layers'] = layer_num


with open(target_yaml, 'r') as file:
    data = yaml.safe_load(file)

if run_mode == 'pretrain':
    write_pretrain_yaml(data)
elif run_mode == "finetune":
    write_finetune_yaml(data)
else:
    raise RuntimeError(f"run_mode{run_mode} not valid!")


