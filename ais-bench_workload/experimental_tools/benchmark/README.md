# AISBench benchmark评测工具
## 简介
AISBench benchmark评测工具是基于opencompass开发的评测工具，兼容opencompass的配置文件、数据集、模型后端等具体实现。目前支持评测推理精度。

## 工具安装
AISBench benchmark是纯python开发的工具，要求`python == 3.10`
本工具的依赖较多，推荐在conda虚拟环境下安装。
```shell
conda create --name ais_bench python=3.10 -y
conda activate ais_bench
```

目前只支持源码构建安装，请确保安装环境网络畅通：

```shell
git clone https://gitee.com/ascend/tools.git
cd tools/ais-bench_workload/experimental_tools/benchmark
pip3 install -e ./
```
安装过程中会自动安装基础依赖。
因为当前工具的模型后端都是服务化api后端，因此需要额外安装服务化的依赖：
```shell
pip3 install -r requirements/api.txt
```

## 数据集准备
从opencompass的release中下载[数据集](https://github.com/open-compass/opencompass/releases/tag/0.2.2.rc1)
`OpenCompassData-core-20240207.zip`已经包含了gsm8k的数据集，解压`OpenCompassData-core-20240207.zip`后将其中的gsm8k文件夹放置到
`ais_bench/datasets/`路径下。

## 快速入门
在本工具的评测中，每个评估任务由待评估的模型后端和数据集组成，可以通过两种方式来指定模型和数据集：命令行指定模型和数据集以及在配置文件中指定模型和数据集。由于当前工具支持的模型后端都是服务化api，请先参考[vllm官方文档/启动服务器样例](https://vllm.hyper.ai/docs/tutorials/vLLM-stepbysteb#%E4%B8%89%E5%90%AF%E5%8A%A8-vllm-%E6%9C%8D%E5%8A%A1%E5%99%A8)在gpu服务器上拉起vllm的推理服务。<br>
### 命令行指定模型和数据集
命令行方式指定模型和数据集本质上是调用工具内置的.py配置文件指定的，需要先在`ais_bench/benchmark/configs/models/`中预置的模型配置文件中配置好服务化相关参数，例如我要执行vllm_api_llama3_8b的任务，需要在`ais_bench/benchmark/configs/models/vllm_api/vllm_api_general.py`中修改配置：

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
        host_ip = "localhost", # 指定服务化的 host ip
        host_port = 8080, # 指定服务化的端口
        enable_ssl = False,
        max_out_len=512,
    )
]
```
修改好配置文件后，执行如下命令启动评测：
```
ais_bench --models vllm_api_general --datasets gsm8k_gen
```

### 配置文件指定模型和数据集
需要先在源码中提供的样例配置文件`ais_bench/configs/`中预置的模型配置文件中配置好服务化相关参数，例如我要执行的配置文件是`ais_bench/configs/api_examples/infer_api_vllm_general.py`中修改配置：

```python
from mmengine.config import read_base
from ais_bench.benchmark.models import VLLMCustomAPI
from ais_bench.benchmark.partitioners import NaivePartitioner
from ais_bench.benchmark.runners.local_api import LocalAPIRunner
from ais_bench.benchmark.tasks import OpenICLInferTask

with read_base():
    # from ais_bench.benchmark.configs.datasets.collections.chat_medium import datasets
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
ais_bench ais_bench/configs/api_examples/infer_api_vllm_general.py
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

## 完整命令行说明

|命令行参数|简介|样例|
| ----- | ----- | ---- |
|config|启动用的配置文件路径(.py)，可以不加，<b>但一定是整个命令行第一个参数</b>|ais_bench xxx/yyy.py {其他可选命令}|
|--models|指定模型任务名称（对应ais_bench/benchmark/configs/models路径下一个已经实现的默认模型配置文件），支持传入多个任务名称|--models vllm_api_llama3_8b|
|--datasets|指定数据集任务名称（对应ais_bench/benchmark/configs/datasets路径下一个已经实现的默认数据集配置文件）|--datasets gsm8k_gen|
|--summarizer|指定数据集汇总任务名称（对应ais_bench/benchmark/configs/summarizers路径下一个已经实现的默认模型配置文件）|--summarizer medium|
|--debug|debug模式开关，加或者不加|--debug|
|--dry-run|dry run模式（只打屏不实际跑任务）开关，加或者不加|--dry-run|
|--mode/-m|可选["all", "infer", "eval", "viz"]，默认"all"，当前两种模式效果完全一样|--mode infer <br>-m all|
|--reuse/-r|重复使用的时间戳文件夹，寻找--work_dir指定的工作路径下最新的文件夹|--reuse <br>-r 20250126_144254|
|--work-dir/-w|工作路径，默认outputs/default| --work-dir /path/to/work <br>-w /path/to/work|
|--config-dir|搜索默认models，datasets和summarizers的文件夹路径， 默认ais_bench/benchmark/configs|--config-dir /xxx/xxx|
|--max-num-workers|并行运行的worker的最大个数，默认1|--max-num-workers 1|
|--max-workers-per-gpu|评测需要用到的npu或gpu数量，当前评测只能使用cpu，此参数无实际效果。此参数默认取值为1|--max-workers-per-gpu 1|
|--dump-eval-details|是否dump出评测过程的细节，开关|--dump-eval-details|
|--dump-extract-rate|是否dump出评测速度，开关|--dump-extract-rate|