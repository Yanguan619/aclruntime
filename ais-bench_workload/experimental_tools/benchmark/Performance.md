# AISBench benchmark性能测评场景
AISBench执行性能测评需指定-mode为perf，以在npu上部署的mindie推理服务上评测gsm8k数据集的性能为例，请先参考[昇腾文档/启动MindIE Service服务](https://www.hiascend.com/document/detail/zh/mindie/100/mindieservice/servicedev/mindie_service0004.html)在npu服务器上拉起mindie的推理服务。<br>
注意：当前AISBench性能测评仅支持mindie服务后端模型
### gsm8k数据集准备
参考[gsm8k数据集说明](ais_bench/benchmark/configs/datasets/gsm8k/README.md)准备数据集，将数据集放在ais_bench/datasets路径下。

### 命令行指定模型后端和数据集
命令行方式指定模型和数据集本质上是调用工具内置的.py配置文件指定的，需要先在`ais_bench/benchmark/configs/models/`中预置的模型配置文件中配置好服务化相关参数，以执行mindie_stream_api的任务为例，需要在`ais_bench/benchmark/configs/models/mindie_api/mindie_stream_api_general.py`中修改配置：

```python
from ais_bench.benchmark.models import MindieStreamApi

models = [
    dict(
        type=MindieStreamApi,
        path="xxx", # 性能测评需指定tokenizer文件路径
        abbr='mindie-stream-api',
        max_seq_len = 4096,
        query_per_second = 1,
        rpm_verbose = False,
        retry = 2,
        host_ip = "localhost", # 推理服务的IP
        host_port = 8080, # 推理服务的端口
        enable_ssl = False,
        max_out_len = 512, # 最大输出tokens长度
        generation_kwargs = dict( # 后处理参数参考https://www.hiascend.com/document/detail/zh/mindie/100/mindieservice/servicedev/mindie_service0090.html 中的parameters的子参数
            temperature = 0.5,
            top_k = 10,
            top_p = 0.95,
            do_sample = True,
            seed = None,
            repetition_penalty = 1.03,
            details = True,
            typical_p = 0.5,
            watermark = False,
            priority = 5,
            timeout = None,
        )
    )
]
```
修改好配置文件后，执行如下命令启动评测
```
ais_bench --models mindie_stream_api_general --datasets gsm8k_gen --mode perf 
```

### 配置文件指定模型后端和数据集
按实际需要自行构建或选择`ais_bench/configs/api_examples`中预置的模型配置文件并配置好服务化相关参数，例如要执行的配置文件是`ais_bench/configs/api_examples/infer_mindie_stream_api_general.py`，需要在此配置文件中修改配置：

```python
from mmengine.config import read_base
from ais_bench.benchmark.models import MindieStreamApi
from ais_bench.benchmark.partitioners import NaivePartitioner
from ais_bench.benchmark.runners.local_api import LocalAPIRunner
from ais_bench.benchmark.tasks import OpenICLInferTask

with read_base():
    from ais_bench.benchmark.configs.summarizers.example import summarizer
    from ais_bench.configs.api_examples.all_dataset_configs import *

datasets = [ # all_dataset_configs.py中导入了其他数据集配置，可以将gsm8k_0_shot_cot_str替换为其他一个或多个数据集
    *gsm8k_0_shot_cot_str,
]


models = [
    dict(
        type=MindieStreamApi,
        path="xxx", #性能测评需指定tokenizer文件路径
        abbr='mindie-stream-api-general',
        max_seq_len = 4096,
        query_per_second = 1,
        rpm_verbose = False,
        retry = 2,
        host_ip = "localhost", # 推理服务的IP
        host_port = 8080, # 推理服务的端口
        enable_ssl = False,
        max_out_len = 512, # 最大输出tokens长度
        generation_kwargs = dict( # 后处理参数参考https://www.hiascend.com/document/detail/zh/mindie/100/mindieservice/servicedev/mindie_service0090.html 中的parameters的子参数
            temperature = 0.5,
            top_k = 10,
            top_p = 0.95,
            max_new_tokens = 512,
            do_sample = True,
            seed = None,
            repetition_penalty = 1.03,
            details = True,
            typical_p = 0.5,
            watermark = False,
            priority = 5,
            timeout = None,
        )
    )
]


infer = dict(partitioner=dict(type=NaivePartitioner),
             runner=dict(
                 type=LocalAPIRunner,
                 max_num_workers=2,
                 concurrent_users=2,
                 task=dict(type=OpenICLInferTask)), )

work_dir = 'outputs/api-mindie-stream/' # 工作路径
```
修改好配置文件后，执行如下命令启动评测：
```
ais_bench ais_bench/configs/api_examples/infer_mindie_stream_api_general.py --mode perf
```

### 落盘结果说明

以synthetic数据集为例，生成目录结构如下：
```bash
outputs/default/
├── 20200220_120000
├── 20230220_183030     # 每个实验一个文件夹
│   ├── configs         # 用于记录的已转储的配置文件。如果在同一个实验文件夹中重新运行了不同的实验，可能会保留多个配置
│   ├── logs            # 推理阶段的日志文件
│   │   └── performance
│   └── performance       # 性能测评结果
│       └── mindie_stream_api  #后端模型，以mindie_stream_api为例
│           └── synthetic      #数据集，以synthetic数据集为例
│               ├── synthetic.csv     #文件1：单个推理请求性能输出结果
│               └── synthetic.json    #文件2：端到端性能输出结果
├── ...
```

