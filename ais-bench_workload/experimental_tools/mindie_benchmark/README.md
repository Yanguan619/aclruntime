# MindIE benchmark评测工具
## 简介
针对MindIE-LLM推理后端，AISBench推出MindIE benchmark测试脚本用于提供昇腾纯模型推理测评能力。目前支持单机/多机拉起纯模型数据集精度和性能测评（注：当前AISBench不支持同时测评精度和性能）

## 环境安装

MindIE benchmark工具依赖MindIE提供推理能力，以及AISBench benchmark提供拉起测评的能力，需要提前准备好上述两个环境。

工具的使用需要拉取源码：
```shell
git clone https://gitee.com/ascend/tools.git
cd tools/
git checkout develop
cd ais-bench_workload/experimental_tools/mindie_benchmark # mindie_llm_examples文件夹内存储着拉起MindIE-LLM纯模型后端测评的参数配置文件
```

参考下列指导进行依赖环境的安装：

MindIE容器安装昇腾社区文档：[拉取镜像方式安装MIndIE](https://www.hiascend.com/document/detail/zh/mindie/100/envdeployment/instg/mindie_instg_0021.html)。

注：由于AISBench benchmark声明支持`python == 3.10`，而MindIE容器仅提供了`python == 3.11`版本，需要在执行AISBench的安装前在MIndIE容器中执行以下命令安装额外依赖，安装完依赖后，在容器内进行AISBench benchmark工具的安装即可，无需构造conda环境。
```shell
# 在MIndIE容器中额外安装指定版本的pyext包
pip3 install pyext==0.5
```

AISBench benchmark安装参考：[AISBench benchmark安装方法](https://gitee.com/aisbench/benchmark/blob/master/README.md#%E5%B7%A5%E5%85%B7%E5%AE%89%E8%A3%85)


## 完整命令说明

### 命令格式说明
```shell
ASCEND_RT_VISIBLE_DEVICES=<device_id> ais_bench <config_file_path>
```
参数说明：
- `ASCEND_RT_VISIBLE_DEVICES=<device_id>`用于配置使用昇腾设备具体卡号
- <config_file_path>为ais_bench工具启动用的配置文件，例如：ais_bench mindie_llm_examples/infer_mindie_llm_general.py，需指定该文件相对于当前文件夹的相对路径或者该文件的绝对路径
- ais_bench命令行参数可参考[AISBench benchmark参数说明](https://gitee.com/aisbench/benchmark/blob/master/README.md#%E5%8F%82%E6%95%B0%E8%AF%B4%E6%98%8E)

### 模式控制参数说明

以下参数在配置文件中用于设定控制AISBench benchmark测评模式。

|参数|说明|默认值|取值范围|
| ----- | ----- | ----- | ----- |
|max_out_len|调用推理接口设定的最大输出长度，建议大小不超过MindIE-LLM推理后端参数output_length|1024，表示最长支持输出1024个token|正整数|
|num_gpus|当前机器下选择使用几张卡进行推理测评任务|2，表示使用两张卡，具体卡号可使用`ASCEND_RT_VISIBLE_DEVICES`设定|[1, 总卡数]|
|num_procs|当前机器下拉起的进程数，需要与卡数相同|2，表示在两张卡上拉起两个进程|[1, 总卡数]|
|nnodes|选择使用的机器个数，配置大于1的参数用于多机测评场景|1，表示使用单机拉起测评任务|正整数|
|node_rank|多机测评时，当前机器的id，主节点id为0，其他节点id的顺序需要与[`rank_table_file`文件](#多机数据集精度测评)中顺序对应|0，表示是主节点（单机场景下不生效）|[0, 总机器个数)|
|master_addr|多机测评时，主节点的ip地址|localhost，当前机器（单机场景下不生效）|具体ip地址|
|enable_detail_perf|是否开启性能测评模式，dump详细的pa runner每个batch的性能数据，--perf模式下才有意义 参考[模式说明](https://gitee.com/aisbench/benchmark/blob/master/README.md#%E8%BF%90%E8%A1%8C%E6%A8%A1%E5%BC%8F%E8%AF%B4%E6%98%8E)|False，表示不开启性能测评模式，不dump详细性能数据|True或Flase|
|input_token_len|性能测评模式下期望用于模型推理的长度，建议不超过MindIE-LLM推理后端参数input_length|16，表示在性能场景下最长构造16个token的输入数据|正整数|


## 使用场景说明

MIndIE benchmark支持单机和多机的精度和性能测评，改动配置文件内的参数就可切换对应的模式。配置文件中的参数主要分成两部分：[AISBench模式控制](#模式控制参数说明)和MIndIE-LLM模型推理配置参数。MIndIE配置参数用于透传给MIndIE推理后端，当前提供单机和多机拉起模型测评的参数配置样例和说明。下面会对各种场景下需要修改的常见参数进行举例说明。

### 单机数据集精度测评

单机场景下拉起任务的指令示例：
```shell
ASCEND_RT_VISIBLE_DEVICES=0,1 ais_bench mindie_llm_examples/infer_mindie_llm_general.py
```

以下配置文件中，有几点需要注意：

- 测评的数据集需要在`datasets`中配置对应的数据集。all_dataset_configs.py可查看可配置的[数据集配置文件](https://gitee.com/aisbench/benchmark/blob/master/ais_bench/configs/api_examples/all_dataset_configs.py)
- `world_size`需要与单机场景下使用的总卡数相同
- `model_name`配置对应权重的模型名称
- `data_type`表示模型推理过程的数据精度，需要与模型权重的精度相同
- `weight_dir`需要设定具体的权重路径
- `decode_batch_size`表示decode阶段的batchsize大小，需要与数据集配置文件中设定的`batch_size`相同
- `input_length`用于初始化推理对象实例，并在推理过程中起到申请内存的功能，建议根据数据集实际情况设定
- `output_length`用于初始化推理对象实例，并在推理过程中起到申请内存的功能，建议根据数据集实际情况设定
- `environ_kwargs`是MindIE-LLM推理后端在具体模型和数据集推理时设定的一些环境变量，不同场景下会略有不同，此处仅做透传,设定之后，会在加载权重前设定好对应的环境变量


**配置文件参数设定样例：**
```python
from mmengine.config import read_base
from ais_bench.benchmark.models import MindieLLMModel

with read_base():
    from ais_bench.benchmark.configs.summarizers.example import summarizer
    from ais_bench.benchmark.configs.datasets.gsm8k.gsm8k_gen_0_shot_cot_str import gsm8k_datasets as gsm8k_0_shot_cot_str

datasets = [ # all_dataset_configs.py中导入了其他数据集配置，可以将gsm8k_0_shot_cot_str替换为其他数据集
    *gsm8k_0_shot_cot_str,
]


models = [
    dict(
        ## 下列参数用于控制AISBench benchmark工具实现功能
        type=MindieLLMModel,
        attr="local", 
        abbr='mindie-llm-api',
        max_out_len = 1024, 
        run_cfg = dict( 
            num_gpus=2, 
            num_procs=2,
            nnodes=1, 
            node_rank=0, 
            master_addr="localhost",
            ),
        enable_detail_perf = False,
        input_token_len = 16, 


        ## 下列参数是用于拉起MindIE-LLM推理后端的参数，用于透传给MindIE-LLM后端，具体功能和含义由用户保证
        world_size = 2,  # 本次推理使用的卡总数
        block_size = 128,  # 初始化推理对象所需参数，预先计算内存所需的参数
        model_name = "qwen",  # 模型名称
        data_type = "bf16",  # 模型配置数据类型
        weight_dir = "/data/Qwen2.5-7B-Instruct",  # 模型权重路径
        max_position_embedding = -1,  # 初始化推理对象所需参数，-1表示使用input_length + output_length
        is_chat_model = False,  # 是否使用chat模板
        decode_batch_size = 32,  # decode阶段的batchsize，需要与数据集测评任务中设定的batch_size相同
        prefill_batch_size = 0,  # prefill阶段的batchsize
        kw_args = "",
        trust_remote_code = False,  # 是否信任远端代码
        ignore_eos = False,  # 是否忽略推理终止符；设置了enable_detail_perf情况下,ignore_eos强制开启
        input_length = 2048,  # 初始化推理对象参数，input长度
        output_length = 1024,  # 初始化推理对象参数，output长度

        dp = -1,  # dp tp sp moe_tp pp microbatch_size moe_ep 模型并行策略参数
        tp = -1,
        sp = -1,
        moe_tp = -1,
        moe_ep = -1,
        pp = -1,
        microbatch_size = -1,

        rank_table_file = "",  # 多机模式下，rank_table路径
        
        environ_kwargs = dict(  # mindie-llm推理后端所需的环境变量配置, 具体模型有对应所需的环境变量
            ATB_LAYER_INTERNAL_TENSOR_REUSE = "1",
            ATB_OPERATION_EXECUTE_ASYNC = "1",
            ATB_CONVERT_NCHW_TO_ND = "1",
            TASK_QUEUE_ENABLE = "1",
            ATB_WORKSPACE_MEM_ALLOC_GLOBAL = "1",
            ATB_CONTEXT_WORKSPACE_SIZE = "0",
            ATB_LAUNCH_KERNEL_WITH_TILING = "1",
            ATB_LLM_ENABLE_AUTO_TRANSPOSE = "0",
            PYTORCH_NPU_ALLOC_CONF = "expandable_segments:True",
            LCCL_DETERMINISTIC = "1",
            HCCL_DETERMINISTIC = "true",
            ATB_MATMUL_SHUFFLE_K_ENABLE = "0",
            # ENABLE_GREEDY_SEARCH_OPT = "0",   # BoolQ数据数据集精度测评环境变量
        ),
    )
]

work_dir = 'outputs/mindie-llm-model/' # 工作路径
```

### 多机数据集精度测评

多机场景下拉起任务时，需要在每个机器上都配置好AISBench运行环境以及运行对应指令示例：
```shell
# 主节点，执行infer和eval的任务（主节点仅有一个）
ASCEND_RT_VISIBLE_DEVICES=0,1,2,3,4,5,6,7 ais_bench mindie_llm_examples/infer_mindie_llm_general.py
# 副节点，仅执行infer任务（副节点可以有多个，执行指令相同） --mode infer 表示仅进行推理过程，不评测，评测由主节点进行
ASCEND_RT_VISIBLE_DEVICES=0,1,2,3,4,5,6,7 ais_bench mindie_llm_examples/infer_mindie_llm_general.py --mode infer
```

以下配置文件中，有几点需要注意：

- 测评的数据集需要在`datasets`中配置对应的数据集。all_dataset_configs.py可查看可配置的[数据集配置文件](https://gitee.com/aisbench/benchmark/blob/master/ais_bench/configs/api_examples/all_dataset_configs.py)
- 多机参数`run_cfg`有对应改动，详细说明可见[模式控制参数说明](#模式控制参数说明)
- `world_size`表示总卡数，是所有机器使用的卡数之和
- `model_name`配置对应权重的模型名称
- `data_type`表示模型推理过程的数据精度，需要与模型权重的精度相同
- `weight_dir`需要设定具体的权重路径
- `is_chat_model`是否使用chat模板（Deepseek-R1模型建议开启）
- `decode_batch_size`表示decode阶段的batchsize大小，需要与数据集配置文件中设定的`batch_size`相同
- `input_length`用于初始化推理对象实例，并在推理过程中起到申请内存的功能，建议根据数据集实际情况设定
- `output_length`用于初始化推理对象实例，并在推理过程中起到申请内存的功能，建议根据数据集实际情况设定
- `dp tp sp moe_tp pp microbatch_size moe_ep`MindIE-LLM推理后端所需的并行策略参数
- `environ_kwargs`是MindIE-LLM推理后端在具体模型和数据集推理时设定的一些环境变量，不同场景下会略有不同，此处仅做透传,设定之后，会在加载权重前设定好对应的环境变量
- `rank_table_file`提供ranktable文件路径，存储分布式拉起任务的集群信息

**rank_table_file构建**

（1）查看8卡ip
```shell
for i in {0..7};do hccn_tool -i $i -ip -g; done
```
（2）若没有配置8卡ip，按以下步骤自定义卡ip (需将10.20.3.13*替换为实际IP)
```shell
for i in {0..7}; do hccn_tool -i ${i} -ip -s address 10.20.3.13${i} netmask 255.255.255.0; done
```
（3）将上述ip地址配置到具体ranktable文件中，文件格式和内容可查看[ranktable文件配置资源信息](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/81RC1alpha001/devguide/hccl/hcclug/hcclug_000014.html)


**配置文件参数设定样例：**

```python
from mmengine.config import read_base
from ais_bench.benchmark.models import MindieLLMModel

with read_base():
    from ais_bench.benchmark.configs.summarizers.example import summarizer
    from ais_bench.benchmark.configs.datasets.gsm8k.gsm8k_gen_0_shot_cot_str import gsm8k_datasets as gsm8k_0_shot_cot_str

datasets = [ # all_dataset_configs.py中导入了其他数据集配置，可以将gsm8k_0_shot_cot_str替换为其他一个或多个数据集
    *gsm8k_0_shot_cot_str,
]


models = [
    dict(
        ## 下列参数用于控制AISBench benchmark工具实现功能
        type=MindieLLMModel,
        attr="local", 
        abbr='mindie-llm-api',
        max_out_len = 15360,  
        run_cfg = dict( 
            num_gpus=8,  
            num_procs=8, 
            nnodes=2,      
            node_rank=0,   
            master_addr="localhost",   
            ),
        enable_detail_perf = False, 
        input_token_len = 16, 


        ## 下列参数是用于拉起MindIE-LLM推理后端的参数，用于透传给MindIE-LLM后端，具体功能和含义由用户保证
        world_size = 16,  # 本次推理使用的卡总数
        block_size = 128,  # 初始化推理对象所需参数，预先计算内存所需的参数
        model_name = "deepseek",  # 模型名称
        data_type = "fp16",  # 模型配置数据类型
        weight_dir = "/data/DeepSeek-R1_w8a8",  # 模型权重路径
        max_position_embedding = -1,  # 初始化推理对象所需参数，-1表示使用input_length + output_length
        is_chat_model = True,  # 是否使用chat模板
        decode_batch_size = 32,  # decode阶段的batchsize，需要与数据集测评任务中设定的batch_size相同
        prefill_batch_size = 0,  # prefill阶段的batchsize
        kw_args = "",
        trust_remote_code = False,  # 是否信任远端代码
        ignore_eos = False,  # 是否忽略推理终止符；设置了enable_detail_perf情况下,ignore_eos强制开启
        input_length = 2048,  # 初始化推理对象参数，input长度
        output_length = 15360,  # 初始化推理对象参数，output长度

        dp = 4,  # dp tp sp moe_tp pp microbatch_size moe_ep 模型并行策略参数
        tp = 4,
        sp = -1,
        moe_tp = 1,
        moe_ep = 16,
        pp = -1,
        microbatch_size = -1,

        rank_table_file = "",  # 多机模式下，rank_table路径
        
        environ_kwargs = dict(  # mindie-llm推理后端所需的环境变量配置, 具体模型有对应所需的环境变量
            ATB_LAYER_INTERNAL_TENSOR_REUSE = "1",
            ATB_OPERATION_EXECUTE_ASYNC = "1",
            ATB_CONVERT_NCHW_TO_ND = "1",
            TASK_QUEUE_ENABLE = "1",
            ATB_WORKSPACE_MEM_ALLOC_GLOBAL = "1",
            ATB_CONTEXT_WORKSPACE_SIZE = "0",
            ATB_LAUNCH_KERNEL_WITH_TILING = "1",
            ATB_LLM_ENABLE_AUTO_TRANSPOSE = "0",
            PYTORCH_NPU_ALLOC_CONF = "expandable_segments:True",
            LCCL_DETERMINISTIC = "1",
            HCCL_DETERMINISTIC = "true",
            ATB_MATMUL_SHUFFLE_K_ENABLE = "0",
            # ENABLE_GREEDY_SEARCH_OPT = "0",   # BoolQ数据数据集精度测评环境变量
        ),
    )
]

work_dir = 'outputs/mindie-llm-model/' # 工作路径
```

### 性能测评

MIndIE benchmark工具提供了单机数据集性能测评功能，用户只需配置好数据集、模型、推理参数等信息，即可快速进行数据集性能测评。

单机场景下拉起任务的指令示例：
```shell
cd ais-bench_workload/experimental_tools/mindie_benchmark
python mindie_llm.py --config mindie_llm_examples/infer_mindie_llm_general.py --batch_size 1 --case_pair [[256,256]] --dataset_path /data/gsm8k --output_path /home/output
```
命令行参数说明：
|参数|说明|默认值|
| ----- | ----- | ----- |
|--config|Ais-bench配置文件路径，可以根据用户的实际情况修改|ais-bench_workload/experimental_tools/mindie_benchmark/mindie_llm_examples/infer_mindie_llm_general.py|
|--batch_size|数据集的batch_size大小。batch_size支持单个输入，如16或[16]；多个输入，如16,32或[16,32]；多组输入，如[[16,32],[32,64]]，此时组数应与case_pair的组数相同|16|
|--case_pair|输入长度和输出长度的组合，如[[256,256]]表示输入长度为256，输出长度为256。case_pair接收一组或多组输入，格式为[[seq_in_1,seq_out_1]...,[seq_in_n,seq_out_n]],中间不接受空格|[[2048,2048],[1024,1024],[512,512],[256,256]]|
|--dataset_path|真实数据集路径。dataset_path需要用户自行准备数据集，并传入数据集路径|无|
|--output_path|性能评测结果输出路径|当前目录|


**配置文件参数设定样例：**
```python
from mmengine.config import read_base
from ais_bench.benchmark.models import MindieLLMModel

with read_base():
    from ais_bench.benchmark.configs.summarizers.example import summarizer
    from ais_bench.benchmark.configs.datasets.gsm8k.gsm8k_gen_0_shot_cot_str_perf import gsm8k_datasets as gsm8k_0_shot_cot_str

datasets = [ # 在ais_bench/configs/api_sample/all_dataset_configs.py中导入了其他数据集配置，可以将gsm8k_0_shot_cot_str替换为其他数据集
    *gsm8k_0_shot_cot_str,
]


models = [
    dict(
        ## 下列参数用于控制AISBench benchmark工具实现功能
        type=MindieLLMModel,
        attr="local", 
        abbr='mindie-llm-api',
        max_out_len = 1024, 
        run_cfg = dict( 
            num_gpus=2, 
            num_procs=2,
            nnodes=1, 
            node_rank=0, 
            master_addr="localhost",
            ),
        enable_detail_perf = True,
        input_token_len = 16, 


        ## 下列参数是用于拉起MindIE-LLM推理后端的参数，用于透传给MindIE-LLM后端，具体功能和含义由用户保证
        world_size = 2,  # 本次推理使用的卡总数
        block_size = 128,  # 初始化推理对象所需参数，预先计算内存所需的参数
        model_name = "qwen",  # 模型名称
        data_type = "bf16",  # 模型配置数据类型
        weight_dir = "/data/Qwen2.5-7B-Instruct",  # 模型权重路径
        max_position_embedding = -1,  # 初始化推理对象所需参数，-1表示使用input_length + output_length
        is_chat_model = False,  # 是否使用chat模板
        decode_batch_size = 32,  # decode阶段的batchsize，需要与数据集测评任务中设定的batch_size相同
        prefill_batch_size = 0,  # prefill阶段的batchsize
        kw_args = "",
        trust_remote_code = False,  # 是否信任远端代码
        ignore_eos = False,  # 是否忽略推理终止符；设置了enable_detail_perf情况下,ignore_eos强制开启
        input_length = 2048,  # 初始化推理对象参数，input长度
        output_length = 1024,  # 初始化推理对象参数，output长度

        dp = -1,  # dp tp sp moe_tp pp microbatch_size moe_ep 模型并行策略参数
        tp = -1,
        sp = -1,
        moe_tp = -1,
        moe_ep = -1,
        pp = -1,
        microbatch_size = -1,

        rank_table_file = "",  # 多机模式下，rank_table路径
        
        environ_kwargs = dict(  # mindie-llm推理后端所需的环境变量配置, 具体模型有对应所需的环境变量
            ATB_LAYER_INTERNAL_TENSOR_REUSE = "1",
            ATB_OPERATION_EXECUTE_ASYNC = "1",
            ATB_CONVERT_NCHW_TO_ND = "1",
            TASK_QUEUE_ENABLE = "1",
            ATB_WORKSPACE_MEM_ALLOC_GLOBAL = "1",
            ATB_CONTEXT_WORKSPACE_SIZE = "0",
            ATB_LAUNCH_KERNEL_WITH_TILING = "1",
            ATB_LLM_ENABLE_AUTO_TRANSPOSE = "0",
            PYTORCH_NPU_ALLOC_CONF = "expandable_segments:True",
            LCCL_DETERMINISTIC = "1",
            HCCL_DETERMINISTIC = "true",
            ATB_MATMUL_SHUFFLE_K_ENABLE = "0",
            # ENABLE_GREEDY_SEARCH_OPT = "0",   # BoolQ数据数据集精度测评环境变量
        ),
    )
]

work_dir = 'outputs/mindie-llm-model/' # 工作路径
```

**性能测评结果**

性能测评结果输出路径下，会生成性能测评结果的csv文件，文件名为performance_pa_batch{batch_size}_tp{world_size}_result.csv。

| 字段                     | 含义                                                         |
| ------------------------ | ------------------------------------------------------------ |
| Model                 | 模型名称        |
| Batchsize           | 数据集的batch_size大小 |
| In_seq       | 推理输入长度 |
| Out_seq           | 推理输出长度  |
| Total time(s)       | 推理总时长 |
| First token time(ms)       | 首token时间 |
| Non-first token time(ms)   | 非首token时间 |
| Non-first token Throughput(Token/s)       | 非首token吞吐量 |
| Throughput(Token/s)       | 吞吐量 |
| Non-first token Throughput Average(Token/s)   | 非首token平均吞吐量 |
| E2E Throughput Average(Token/s)           | 平均吞吐量  |
