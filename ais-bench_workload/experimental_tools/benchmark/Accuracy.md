# AISBench benchmark精度测评场景
AISBench默认执行精度测评，以在gpu上部署的vllm推理服务上评测gsm8k数据集的精度为例，请先参考[vllm官方文档/启动服务器样例](https://vllm.hyper.ai/docs/tutorials/vLLM-stepbysteb#%E4%B8%89%E5%90%AF%E5%8A%A8-vllm-%E6%9C%8D%E5%8A%A1%E5%99%A8)在gpu服务器上拉起vllm的推理服务。<br>
### gsm8k数据集准备
参考[gsm8k数据集说明](ais_bench/benchmark/configs/datasets/gsm8k/README.md)准备数据集，将数据集放在ais_bench/datasets路径下。

### 命令行指定模型后端和数据集
命令行方式指定模型和数据集本质上是调用工具内置的.py配置文件指定的，需要先在`ais_bench/benchmark/configs/models/`中预置的模型配置文件中配置好服务化相关参数，以执行vllm_api_general的任务为例，需要在`ais_bench/benchmark/configs/models/vllm_api/vllm_api_general.py`中修改配置：

```python
from ais_bench.benchmark.models import VLLMCustomAPI

models = [
    dict(
        type=VLLMCustomAPI,
        abbr='vllm-api-general',
        max_seq_len = 4096,
        query_per_second = 1,
        rpm_verbose = False,
        retry = 2,
        host_ip = "localhost",  # 指定服务化的 host ip
        host_port = 8080,  # 指定服务化的端口
        enable_ssl = False,
        generation_kwargs = dict(
            temperature = 0.5,
            top_k = 10,
            top_p = 0.95,
            seed = None,
            repetition_penalty = 1.03,
        )
    )
]
```
修改好配置文件后，执行如下命令启动评测
```
ais_bench --models vllm_api_general --datasets gsm8k_gen
```

### 配置文件指定模型后端和数据集
按实际需要自行构建或选择`ais_bench/configs/api_examples`中预置的模型配置文件并配置好服务化相关参数，例如要执行的配置文件是`ais_bench/configs/api_examples/infer_vllm_api_general.py`，需要在此配置文件中修改配置：

```python
from mmengine.config import read_base
from ais_bench.benchmark.models import VLLMCustomAPI
from ais_bench.benchmark.partitioners import NaivePartitioner
from ais_bench.benchmark.runners.local_api import LocalAPIRunner
from ais_bench.benchmark.tasks import OpenICLInferTask

with read_base():
    from ais_bench.benchmark.configs.summarizers.medium import summarizer
    from ais_bench.benchmark.configs.datasets.gsm8k.gsm8k_gen import gsm8k_datasets

datasets = [
    *gsm8k_datasets,
]


models = [
    dict(
        type=VLLMCustomAPI,
        abbr='vllm-api-general',
        max_seq_len = 4096,
        query_per_second = 1,
        rpm_verbose = False,
        retry = 2,
        host_ip = "localhost", # 指定服务化的 host ip
        host_port = 8080, # 指定服务化的端口
        enable_ssl = False,
        max_out_len=512,
    )
]


infer = dict(partitioner=dict(type=NaivePartitioner),
             runner=dict(
                 type=LocalAPIRunner,
                 max_num_workers=2,
                 concurrent_users=2,
                 task=dict(type=OpenICLInferTask)), )

work_dir = 'outputs/api_vllm_general/' # 指定落盘文件（执行过程、推理结果等）的落盘文件夹

```
修改好配置文件后，执行如下命令启动评测：
```
ais_bench ais_bench/configs/api_examples/infer_vllm_api_general.py
```

### 推理过程查看
启动推理过程中可以在{work_dir}/{time_label}/logs/infer/{abbr_name}/gsm8k.out 中查看推理结果，例如执行
```shell
# 命令行指定模型和数据集运行方式
tail -f outputs/default/20250126_165049/logs/infer/vllm-api-general/gsm8k.out

# 配置文件指定模型和数据集运行方式
tail -f outputs/api_vllm_general/20250126_165049/logs/infer/vllm-api-general/gsm8k.out
```
可以看到推理过程。
其中`{work_dir}/{time_label}/`会在工具的打屏中显示

### 推理结果查看
推理完成后可以在{work_dir}/{time_label}/predictions/{abbr_name}/gsm8k.json 中查看推理结果，例如执行
```shell
# 命令行指定模型和数据集运行方式
vim outputs/default/20250126_165049/predictions/vllm-api-general/gsm8k.json

# 配置文件指定模型和数据集运行方式
vim outputs/api_vllm_general/20250126_165049/predictions/vllm-api-general/gsm8k.json
```
可以看到推理结果

### 测评结果查看
在{work_dir}/{time_label}/results/{abbr_name}/gsm8k.json中查看评测出的精度，例如执行
```shell
# 命令行指定模型和数据集运行方式
vim outputs/default/20250126_165049/results/vllm-api-general/gsm8k.json

# 配置文件指定模型和数据集运行方式
vim outputs/api_vllm_general/20250126_165049/results/vllm-api-general/gsm8k.json
```
可以得到类似如下结果：
```json
{
    "accuracy": 59.34
}
```

### 测评结果可视化
评测过程结束后，工具会将markdown格式的结果打印出来，同时会落盘如下三种格式的结果：
```
{work_dir}/{time_label}/summary/summary_{time_label}.txt
{work_dir}/{time_label}/summary/summary_{time_label}.csv
{work_dir}/{time_label}/summary/summary_{time_label}.md
```
例如
```
outputs/api_vllm_general/20250126_165049/summary/summary_20250126_165049.txt
outputs/api_vllm_general/20250126_165049/summary/summary_20250126_165049.csv
outputs/api_vllm_general/20250126_165049/summary/summary_20250126_165049.md
```
