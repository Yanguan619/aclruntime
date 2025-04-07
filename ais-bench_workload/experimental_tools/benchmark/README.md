# AISBench benchmark评测工具
## 简介
AISBench benchmark评测工具是基于opencompass开发的评测工具，兼容opencompass的配置文件、数据集、模型后端等具体实现。目前支持评测不同配置参数下的推理精度或推理性能（注：当前AISbench不支持同时测评精度和性能）。

## 工具安装
AISBench benchmark是由python开发的工具，要求`python == 3.10`
本工具的依赖较多，推荐在conda虚拟环境下安装。
```shell
conda create --name ais_bench python=3.10 -y
conda activate ais_bench
```

目前只支持源码构建安装，请确保安装环境网络畅通：

```shell
git clone https://gitee.com/ascend/tools.git
cd tools/
git checkout develop
cd ais-bench_workload/experimental_tools/benchmark
pip3 install -e ./
```
安装过程中会自动安装基础依赖。
因为当前工具的模型后端都是服务化api后端，因此需要额外安装服务化的依赖：
```shell
pip3 install -r requirements/api.txt
pip3 install -r requirements/extra.txt
```

## 工具卸载
执行命令：
```shell
pip3 uninstall ais_bench_benchmark
```

## 快速入门
在本工具的评测中，每个评估任务由评测模式、待评估的模型后端和数据集组成，可以通过命令行或配置文件两种方式来指定模型后端和数据集。评测模式支持精度测评和性能测评两种模式，精度测评场景详见[精度测评](./Accuracy.md) 性能测评场景详见[性能测评](./Performance.md)

## 完整命令行说明

### 命令格式说明
```shell
ais_bench [OPTIONS]
```
其中[OPTIONS]为ais_bench的可选参数，具体参数如[参数说明](#参数说明)

### 使用示例
```shell
# 命令行方式：gsm8k数据集在mindie后端模型下的性能测评（需配置好数据集和后端模型信息）
ais_bench -m perf --models mindie_stream_api_general --datasets gsm8k_gen
# 配置文件方式：gsm8k数据集在vllm后端模型下的精度测评（需配置好数据集和后端模型信息）
ais_bench ais_bench/configs/api_examples/infer_vllm_api_general.py --debug
```

### 参数说明
|参数|说明|样例|
| ----- | ----- | ---- |
|config|启动用的配置文件路径(.py)，在“配置文件指定模型和数据集”方式中必须配置，与“命令行指定模型和数据集”方式配置--models和--datasets参数二选一，为ais_bench命令行的第一个参数。自定义配置文件可参考[自定义配置文件样例列表](#自定义配置文件样例列表)|ais_bench xxx/yyy.py|
|--models|指定模型推理后端任务名称（对应ais_bench/benchmark/configs/models路径下一个已经实现的默认模型配置文件），支持传入多个任务名称，支持的任务范围请参考[任务支持范围](#任务支持范围)章节<br>此参数在“命令行指定模型和数据集”方式中必须配置，与“配置文件指定模型和数据集”方式中配置的`config` 参数二选一|--models vllm_api_general|
|--datasets|指定数据集任务名称（对应ais_bench/benchmark/configs/datasets路径下一个已经实现的默认数据集配置文件），支持的任务范围请参考[任务支持范围](#任务支持范围)章节<br>此参数在“命令行指定模型和数据集”方式中必须配置，与“配置文件指定模型和数据集”方式中配置的`config`参数二选一|--datasets gsm8k_gen|
|--summarizer|指定结果总结任务名称（对应ais_bench/benchmark/configs/summarizers路径下一个已经实现的默认模型配置文件），支持的任务范围请参考[任务支持范围](#任务支持范围)章节|--summarizer medium|
|--debug|debug模式开关，配置该参数表示开启，未配置表示关闭，默认未配置|--debug|
|--dry-run|dry run模式（只打屏不实际跑任务）开关，配置该参数表示开启，未配置表示关闭，默认未配置|--dry-run|
|--mode 或 -m|可选["all", "infer", "eval", "viz", "perf"]，默认"all"，性能测评需指定为perf，每个模式如何运行参考[运行模式说明](#运行模式说明)|--mode infer <br>-m all|
|--reuse 或 -r|指定重复使用的工作路径下的文件夹时间戳，如果此可选命令不加参数，默认寻找--work_dir指定的工作路径下最新的时间戳|--reuse <br>-r 20250126_144254|
|--work-dir 或 -w|评测任务的工作路径，用于落盘评测过程中的结果文件，默认outputs/default| --work-dir /path/to/work <br>-w /path/to/work|
|--config-dir|models，datasets和summarizers配置文件所在的文件夹路径， 默认ais_bench/benchmark/configs|--config-dir /xxx/xxx|
|--max-num-workers|并行运行的worker的最大个数，默认1|--max-num-workers 1|
|--max-workers-per-gpu|预留参数，暂不支持使用。<br> 评测需要用到的NPU或GPU数量，默认1。|--max-workers-per-gpu 1|
|--dump-eval-details|是否dump出评测过程细节的开关，配置该参数表示开启，未配置表示关闭，默认未配置|--dump-eval-details|
|--dump-extract-rate|是否dump出评测速度的开关，配置该参数表示开启，未配置表示关闭，默认未配置|--dump-extract-rate|
|--help 或 -h|查看当前支持的命令行参数选项|ais_bench -h|

自定义数据集命令行请参考[自定义数据集](#自定义数据集)章节

## 运行模式说明
### all 模式【精度测评】

all模式下评测工具会完整执行一次精度评测流程：
```mermaid
graph LR;
A[基于给定数据集执行推理] --> B((推理结果));
B --> C[基于推理结果测评]
C --> D((精度数据))
D --> E[基于精度数据汇总呈现]
E --> F((呈现结果))
```

命令示例：
```shell
ais_bench --models vllm_api_general --datasets gsm8k_gen --mode all
```

生成结构目录结构：
```bash
outputs/default/
├── 20200220_120000
├── 20230220_183030     # 每个实验一个文件夹
│   ├── configs         # 用于记录的已转储的配置文件。如果在同一个实验文件夹中重新运行了不同的实验，可能会保留多个配置
│   ├── logs            # 推理和评估阶段的日志文件
│   │   ├── eval
│   │   └── infer
│   ├── predictions   # 每个任务的推理结果
│   ├── results       # 每个任务的评估结果
│   └── summary       # 单个实验的汇总评估结果
├── ...
```

### infer模式【精度测评】

infer模式下评测工具仅会跑出数据集的推理结果：
```mermaid
graph LR;
A[基于给定数据集执行推理] --> B((推理结果));
```
命令示例：
```shell
ais_bench --models vllm_api_general --datasets gsm8k_gen --mode infer
```

生成结构目录结构：
```bash
outputs/default/
├── 20200220_120000
├── 20230220_183030     # 每个实验一个文件夹
│   ├── configs         # 用于记录的已转储的配置文件。如果在同一个实验文件夹中重新运行了不同的实验，可能会保留多个配置
│   ├── logs            # 推理和评估阶段的日志文件
│   │   ├── eval
│   │   └── infer
│   ├── predictions   # 每个任务的推理结果
├── ...
```

### eval模式【精度测评】
eval模式下评测工具会基于已有的推理结果跑一遍评测流程和结果呈现的流程，需要结合--reuse命令使用：
```mermaid
graph LR;
B((推理结果)) --> C[基于推理结果测评]
C --> D((精度数据))
D --> E[基于精度数据汇总呈现]
E --> F((呈现结果))
```

命令示例：
```shell
ais_bench --models vllm_api_general --datasets gsm8k_gen --mode eval --reuse
```

生成结构目录结构：
```bash
outputs/default/
├── 20200220_120000
├── 20230220_183030     # 每个实验一个文件夹
│   ├── configs         # 用于记录的已转储的配置文件。如果在同一个实验文件夹中重新运行了不同的实验，可能会保留多个配置
│   ├── logs            # 推理和评估阶段的日志文件
│   │   ├── eval
│   │   └── infer
│   ├── predictions   # 每个任务的推理结果
│   ├── results       # 每个任务的评估结果 (eval新增)
├── ...
```

### viz模式【精度测评】
viz模式下评测工具会基于已有的精度数据跑一遍结果呈现的流程，需要结合--reuse命令使用：
```mermaid
graph LR;
D((精度数据)) --> E[基于精度数据汇总呈现]
E --> F((呈现结果))
```

命令示例：
```shell
ais_bench --models vllm_api_general --datasets gsm8k_gen --mode viz --reuse
```

生成结构目录结构：
```bash
outputs/default/
├── 20200220_120000
├── 20230220_183030     # 每个实验一个文件夹
│   ├── configs         # 用于记录的已转储的配置文件。如果在同一个实验文件夹中重新运行了不同的实验，可能会保留多个配置
│   ├── logs            # 推理和评估阶段的日志文件
│   │   ├── eval
│   │   └── infer
│   ├── predictions   # 每个任务的推理结果
│   ├── results       # 每个任务的评估结果
│   └── summary       # 单个实验的汇总评估结果 (viz新增)
├── ...
```

### perf模式【性能测评】

perf模式下评测工具会完整执行一次性能评测流程：
```mermaid
graph LR;
A[基于给定数据集执行推理] --> B((打点数据));
B --> C[基于性能打点结果计算]
C --> D((性能数据))
D --> E[基于性能数据汇总呈现]
E --> F((呈现结果))
```

命令示例：
```shell
ais_bench --models mindie_stream_api_general --datasets synthetic --mode perf
```

生成结构目录结构：
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
│               ├── synthetic.csv     #单个推理请求性能输出结果
│               └── synthetic.json    #端到端性能输出结果
├── ...
```

## 性能测评结果说明
性能测评结果包括单个推理请求性能输出结果和端到端性能输出结果，参数说明如下：

### 单个推理请求性能输出结果
部分统计指标解释如下所示：
+ P75：以DecodeTime为例，所有请求的DecodeTime的75分位。
+ P90：以DecodeTime为例，所有请求的DecodeTime的90分位。
+ P99：以DecodeTime为例，所有请求的DecodeTime的99分位。
+ Latency：单个请求的时延
+ TTFT（Time To First Token）:首token时延
+ TPOT（Time Per Output Token）：每个输出token的平均时延，请求粒度，不含首token
+ ITL（Inter-token Latency）：token间时延，不含首token
+ InputTokens：输入token长度
+ OutputTokens：输出token长度
+ PrefillTokenThroughput：prefill吞吐率
+ OutputTokenThroughput：output吞吐率
+ Tokenizer：tokenizer时间
+ Detokenizer：detokenizer时间

|Performance Parameters|Average|Max|Min|Median|P75|P90|P99|N|
| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
|Latency|平均请求时延|最大请求时延|最小请求时延|请求时延中位数|请求时延75分位值|请求时延90分位值|请求时延99分位值|测试数据量，来源于输入参数|
|TTFT|首个token平均时延|首个token最大时延|首个token最小时延|首个token中位数时延|首个token75分位时延|首个token90分位时延|首个token99分位时延|测试数据量，来源于输入参数|
|TPOT|Decode阶段平均时延|最大Decode阶段时延|最小Decode阶段时延|Decode阶段中位数时延|75分位Decode阶段时延|90分位每条请求Decode阶段平均时延|99分位Decode阶段时延|测试数据量，来源于输入参数|
|ITL|token间平均时延|token间最大时延|token间最小时延|token间中位数时延|token间75分位时延|token间90分位时延|token间99分位时延|测试数据量，来源于输入参数|
|InputTokens|输入token平均长度|最大输入token长度|最小输入token长度|输入token中位数长度|75分位输入token长度|90分位输入token长度|99分位输入token长度|测试数据量，来源于输入参数|
|OutputTokens|输出token平均长度|最大输出token长度|最小输出token长度|输出token中位数长度|75分位输出token长度|90分位输出token长度|99分位输出token长度|测试数据量，来源于输入参数|
|PrefillTokenThroughput|平均prefill吞吐|最大prefill吞吐|最小prefill吞吐|中位数prefill吞吐|prefill吞吐75分位|prefill吞吐90分位|prefill吞吐99分位|测试数据量，来源于输入参数|
|OutputTokenThroughput|平均输出吞吐|最大输出吞吐|最小输出吞吐|中位数输出吞吐|输出吞吐75分位|输出吞吐90分位|输出吞吐99分位|测试数据量，来源于输入参数|
|Tokenizer|tokenizer的平均时间|tokenizer的最大时间|tokenizer的最小时间|tokenizer的中位数时间|75分位tokenizer处理时间|90分位tokenizer处理时间|99分位tokenizer处理时间|测试数据量，来源于输入参数|
|Detokenizer|detokenizer的平均时间|detokenizer的最大时间|detokenizer的最小时间|detokenizer的中位数时间|75分位detokenizer处理时间|90分位detokenizer处理时间|99分位detokenizer处理时间|测试数据量，来源于输入参数|


### 端到端性能输出结果
|参数|说明|
| ---- | ---- |
|Benchmark Duration|测试总耗时|
|Total Requests|测试数据量|
|Failed Requests|失败请求数据量（包含空和未返回数据的响应）|
|Success Requests|返回请求总数据量（包含非空和空）|
|Concurrency|实际测试并发数|
|Max Concurrency|最大测试并发数|
|Request Throughput|请求吞吐率|
|Total Input Tokens|输入总token数|
|Prefill Token Throughput|prefill吞吐率|
|Total Output Tokens|输出总token数|
|Input Token Throughput|输入吞吐率|
|Output Token Throughput|输出吞吐率|
|Total Token Throughput|总吞吐率|

## 任务支持范围
本节介绍当前ais_bench评测工具支持的评测任务的预设配置，通过ais_bench命令行指定任务名称，即可执行相应的评测任务。
命令示例如下：
```shell
ais_bench --models vllm_api_general --datasets gsm8k_gen --summarizer medium
```
### --models支持的模型推理后端
|任务名称|简介|使用前提|支持的prompt格式(字符串格式或多轮对话)|对应源码配置文件路径|
| --- | --- | --- | --- | --- |
|vllm_api_general|通过vllm兼容OpenAI的api访问vllm(0.6+版本)的推理服务化，访问服务链接的 v1/completions子服务|基于支持v1/completions子服务的vllm版本，启动vllm推理服务|字符串格式|[vllm_api_general.py](ais_bench/benchmark/configs/models/vllm_api/vllm_api_general.py)|
|vllm_api_general_chat|通过vllm兼容OpenAI的api访问vllm(0.6+版本)的推理服务化，访问服务链接的 v1/chat/completions子服务|基于支持v1/chat/completions子服务的vllm版本，启动vllm推理服务|字符串格式、对话格式|[vllm_api_general_chat.py](ais_bench/benchmark/configs/models/vllm_api/vllm_api_general_chat.py)|
|vllm_api_old|通过vllm的api访问vllm(0.2.6版本)的推理服务化，访问服务链接的 generate子服务|基于支持generate子服务的vllm版本，启动vllm推理服务|字符串格式|[vllm_api_old.py](ais_bench/benchmark/configs/models/vllm_api/vllm_api_old.py)|
|mindie_stream_api_general|通过mindie的流式api访问mindie的推理服务化，访问服务链接的 infer子服务|基于支持infer子服务的mindie版本，启动mindie推理服务|字符串格式|[mindie_stream_api_general.py](ais_bench/benchmark/configs/models/mindie_api/mindie_stream_api_general.py)|

**注意:** 服务化推理测评api默认使用的url为localhost，端口号为8080，实际使用时需要修改为服务化后端配置的url和端口号；当前性能测评仅支持mindie_stream_api_general模型后端。

### --datasets支持的数据集
--datasets 支持的数据集如下，每个数据集包含多种数据集任务，数据集的获取方式和支持的数据集任务请参考对应数据集的README。
|数据集|数据集任务README|
| ---- | ---- |
|GSM8K|[ais_bench/benchmark/configs/datasets/gsm8k/README.md](ais_bench/benchmark/configs/datasets/gsm8k/README.md)|
|MMLU|[ais_bench/benchmark/configs/datasets/mmlu/README.md](ais_bench/benchmark/configs/datasets/mmlu/README.md)|
|BoolQ|[ais_bench/benchmark/configs/datasets/SuperGLUE_BoolQ/README.md](ais_bench/benchmark/configs/datasets/SuperGLUE_BoolQ/README.md)|

### --summarizer支持的结果总结任务
|任务名称|简介|对应源码配置文件路径|
| --- | --- | --- |
|medium|通用结果汇总模板，包含多种基本数据集|[medium.py](ais_bench/benchmark/configs/summarizers/medium.py)|

## 自定义配置文件样例列表
|文件名|简介|
| --- | --- |
|[infer_vllm_api_general.py](ais_bench/configs/api_examples/infer_vllm_api_general.py)|基于gsm8k数据集使用vllm兼容OpenAI的api访问v1/completion子服务评测，自定义了数据集路径|
|[infer_mindie_stream_api_general.py](ais_bench/configs/api_examples/infer_mindie_stream_api_general.py)|基于gsm8k数据集使用mindie原生流式api评测，自定义了数据集路径|
|[infer_vllm_api_general_chat.py](ais_bench/configs/api_examples/infer_vllm_api_general_chat.py)|基于gsm8k数据集使用vllm兼容OpenAI的api访问v1/chat/completion子服务评测，自定义了数据集路径|
|[infer_vllm_api_old.py](ais_bench/configs/api_examples/infer_vllm_api_old.py)|基于gsm8k数据集使使用vllm 0.2.6版本格式的api访问generate子评测，自定义了数据集路径|

## 其他特性
### 自定义数据集
参考文档[自定义数据集使用说明](doc/自定义数据集使用说明.md)
