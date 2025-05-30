from mmengine.config import read_base
from ais_bench.benchmark.models import VLLMCustomAPIChat
from ais_bench.benchmark.summarizers import DefaultPerfSummarizer
from mindie_ais_bench_backend.calculators import MindIEPerfMetricCalculator
from mindie_ais_bench_backend.clients import OpenAIChatTextClient

with read_base():
    from ais_bench.benchmark.configs.datasets.synthetic.synthetic_gen import synthetic_datasets
    from ais_bench.benchmark.configs.datasets.gsm8k.gsm8k_gen_0_shot_cot_str_perf import gsm8k_datasets

datasets = [ # all_dataset_configs.py中导入了其他数据集配置，可以将synthetic_datasets替换为其他一个或多个数据集
    *synthetic_datasets,
]

models = [
    dict(
        attr="service", # model or service
        type=VLLMCustomAPIChat,
        abbr='vllm-api-general-chat',
        model="",
        path="",
        request_rate = 0,
        retry = 2,
        host_ip = "xx.xx.xx.xx", # 推理服务的IP
        host_port = 8080, # 推理服务的端口
        enable_ssl = False,
        max_out_len = 10, # 最大输出tokens长度
        batch_size=10, # 推理的最大并发数
        custom_client=dict(type=OpenAIChatTextClient),
        generation_kwargs = dict( # 后处理参数参考https://docs.vllm.ai/en/latest/api/inference_params.html#sampling-params 中的Parameters
            temperature = 0,
            ignore_eos = True,
        )
    )
]

summarizer = dict(
    type=DefaultPerfSummarizer,
    calculator=dict(
        type=MindIEPerfMetricCalculator,
        stats_list=["Average", "Min", "Max", "Median", "P75", "P90", "P99"],
    )
)


work_dir = 'outputs/api-vllm-general-chat/'