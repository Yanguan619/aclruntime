from mmengine.config import read_base
from ais_bench.benchmark.models import VLLMCustomAPIChat
from ais_bench.benchmark.partitioners import NaivePartitioner
from ais_bench.benchmark.runners.local import LocalRunner
from ais_bench.benchmark.tasks import OpenICLInferTask

with read_base():
    from ais_bench.benchmark.configs.summarizers.example import summarizer
    from ais_bench.benchmark.configs.datasets.gsm8k.gsm8k_gen_0_shot_cot_chat_prompt import gsm8k_datasets as gsm8k_0_shot_cot_chat

datasets = [ # all_dataset_configs.py中导入了其他数据集配置，可以将gsm8k_0_shot_cot_chat替换为其他一个或多个数据集
    *gsm8k_0_shot_cot_chat,
]

from ais_bench.benchmark.models import HuggingFaceBaseModel

models = [
    dict(
        type=HuggingFaceBaseModel,
        abbr='hf-base-model',
        path='huggyllama/llama-65b', # path to model dir, current value is just a example
        max_out_len=1024,
        batch_size=8,
        run_cfg=dict(num_gpus=1),
    )
]



infer = dict(partitioner=dict(type=NaivePartitioner),
             runner=dict(
                 type=LocalRunner,
                 max_num_workers=2,
                 max_workers_per_gpu=1,
                 task=dict(type=OpenICLInferTask)), )

work_dir = 'outputs/api-vllm-general-chat/'