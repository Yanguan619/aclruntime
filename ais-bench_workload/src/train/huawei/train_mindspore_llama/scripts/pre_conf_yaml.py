
# should be execute after distribut
import sys
import os
import yaml
import acl

run_mode = sys.argv[1]
cur_path = os.path.dirname(os.path.abspath(__file__))
try:
    soc_version = acl.get_soc_name()
except Exception as err:
    raise RuntimeError("get soc versiob failed!") from err

pretrain_dataset = os.getenv('PRETRAIN_DATA_PATH')
finetune_dataset = os.getenv('FINETUNE_DATA_PATH')

rank_size = int(os.getenv('RANK_SIZE'))
data_parallel = int(os.getenv('DATA_PARALLEL'))
model_parallel = int(os.getenv('MODEL_PARALLEL'))
pipeline_stage = int(os.getenv('PIPELINE_STAGE'))

if not rank_size == data_parallel * model_parallel * pipeline_stage:
    raise RuntimeError("DATA_PARALLEL * MODEL_PARALLEL * PIPELINE_STAGE should equal to RANK_SIZE !")

model_scale = os.getenv('LLAMA_MODEL_SCALE')
model_type = os.getenv('LLAMA_MODEL_TYPE')
config_path = os.path.join(cur_path, f'code/configs/llama{model_type}/')
epoch_size = os.getenv("EPOCH_SIZE")
sink_size = 2
layer_num = os.getenv("LLAMA_LAYER_NUM")
eval_data_path = os.getenv('EVAL_DATASET_PATH')

if 'Ascend 910B'in soc_version:
    target_yaml = os.path.join(config_path, f'run_llama{model_type}_{model_scale}_910b.yaml')
else:
    target_yaml = os.path.join(config_path, f'run_llama{model_type}_{model_scale}.yaml')
if os.getenv('LLAMA_RUN_MODE') == 'only_finetune':
    ckpt_path = os.path.join(cur_path, os.getenv('FINETUNE_CKPT_PATH'))
    if not os.path.exists(ckpt_path):
        raise FileExistsError(f"ckpt_path: {ckpt_path} not find!")
else:
    ckpt_path = os.path.join(cur_path, f'result/output/target_ckpt/rank_0/llama{model_type}_{model_scale}0.ckpt')

if not os.path.exists(target_yaml):
    raise FileExistsError(f"yaml file: {target_yaml} not find!")
if os.path.islink(target_yaml):
    raise RuntimeError(f"yaml file: {target_yaml} is softlink!")

if model_type == "2" and run_mode == "finetune_eval":
    seq_length = 4096
else:
    seq_length = 2048

def change_parallel_params(data):
    data_parallel = int(data['parallel_config']['data_parallel'])
    model_parallel = int(data['parallel_config']['model_parallel'])
    pipeline_stage = int(data['parallel_config']['pipeline_stage'])

    return int(rank_size / (data_parallel * model_parallel * pipeline_stage))

def write_pretrain_yaml(data):
    data['load_checkpoint'] = ''
    data['run_mode'] = 'train'
    data['runner_config']['epochs'] = int(epoch_size)
    data['runner_config']['sink_size'] = sink_size
    data['parallel_config']['data_parallel'] = data_parallel
    data['parallel_config']['model_parallel'] = model_parallel
    data['parallel_config']['pipeline_stage'] = pipeline_stage
    data['optimizer']['beta2'] = 0.95
    data['optimizer']['learning_rate'] = 3.e-4
    data['lr_schedule']['learning_rate'] = 3.e-4
    data['lr_schedule']['lr_end'] = 3.e-5
    data['train_dataset']['input_columns'] = ["input_ids"]
    data['train_dataset']['data_loader']['dataset_dir'] = pretrain_dataset
    data['eval_dataset']['data_loader']['dataset_dir'] = eval_data_path
    data['model']['model_config']['num_layers'] = int(layer_num)
    data['callbacks'][1]['save_checkpoint_steps'] = 100000
    return data


def write_finetune_yaml(data):
    data['load_checkpoint'] = ckpt_path
    data['run_mode'] = 'finetune'
    data['runner_config']['epochs'] = int(epoch_size)
    data['runner_config']['sink_size'] = sink_size
    data['parallel_config']['data_parallel'] = data_parallel
    data['parallel_config']['model_parallel'] = model_parallel
    data['parallel_config']['pipeline_stage'] = pipeline_stage
    data['optimizer']['beta2'] = 0.999
    data['optimizer']['learning_rate'] = 1.e-5
    data['lr_schedule']['learning_rate'] = 1.e-5
    data['lr_schedule']['lr_end'] = 1.e-5
    data['train_dataset']['input_columns'] = ["input_ids", "labels"]
    data['train_dataset']['data_loader']['dataset_dir'] = finetune_dataset
    data['eval_dataset']['data_loader']['dataset_dir'] = eval_data_path
    data['model']['model_config']['num_layers'] = int(layer_num)
    data['model']['model_config']['seq_length'] = seq_length
    data['callbacks'][1]['save_checkpoint_steps'] = 100000
    return data

data = {}
with open(target_yaml, 'r', encoding='utf-8') as file:
    try:
        data = yaml.safe_load(file)
    except Exception as err:
        raise RuntimeError(f"load target_yaml: {target_yaml} failed") from err

if run_mode == 'train':
    data = write_pretrain_yaml(data)
elif run_mode == "finetune":
    data = write_finetune_yaml(data)

with open(target_yaml, 'w', encoding='utf-8') as file:
    try:
        yaml.safe_dump(data, file)
    except Exception as err:
        raise RuntimeError(f"dump target_yaml: {target_yaml} failed") from err

