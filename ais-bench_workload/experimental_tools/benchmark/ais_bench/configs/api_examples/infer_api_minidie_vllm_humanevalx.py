from mmengine.config import read_base
from ais_bench.benchmark.models import VLLMCustomAPI
from ais_bench.benchmark.partitioners import NaivePartitioner
from ais_bench.benchmark.runners.local_api import LocalAPIRunner
from ais_bench.benchmark.tasks import OpenICLInferTask

with read_base():
    # from ais_bench.benchmark.configs.datasets.collections.chat_medium import datasets
    from ais_bench.benchmark.configs.summarizers.example import summarizer
    from ais_bench.benchmark.configs.datasets.humanevalx.humanevalx_gen_0_shot import humanevalx_datasets # 此处导入之前写的数据集配置文件中的数据集实例

datasets = [
    *humanevalx_datasets, # 添加数据集实例
]


models = [
    dict(
        type=VLLMCustomAPI, # 推理后端，建议优先写支持mindie服务化的。VLLMCustomAPIOld可对接mindie服务化(非流式)，VLLMCustomAPI可对接0.6+版本gpu上vllm拉起的服务，MindieStreamApi可对接mindie服务化(非流式)
        abbr='mindie-vllm-api-humanevalx',
        max_seq_len = 4096,
        query_per_second = 1024,
        rpm_verbose = False,
        retry = 2,
        host_ip = "90.91.56.32", # 使用时按实际服务化的ip修改
        max_out_len = 1,

        # 改为本机的
        host_port = 9091, # 使用时按实际服务化的端口修改
        enable_ssl = False,
        generation_kwargs = dict(
            temperature = 0,
            seed = 1,
        )
    )
]


infer = dict(partitioner=dict(type=NaivePartitioner),
             runner=dict(
                 type=LocalAPIRunner,
                 max_num_workers=2,
                 concurrent_users=2,
                 task=dict(type=OpenICLInferTask)), )

work_dir = 'outputs/api-mindie-vllm-humanevalx/' # 自定义的工作路径，工具运行结果会落盘在这个路径下的某个时间戳目录下