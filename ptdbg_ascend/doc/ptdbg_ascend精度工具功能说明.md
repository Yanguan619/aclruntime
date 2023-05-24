# **PyTorch精度工具使用指南**

本文主要介绍PyTorch精度工具精度工具ptdbg_ascend的使用以及精度比对场景示例。

ptdbg_ascend工具的原理及安装请参见《[PyTorch精度工具](https://gitee.com/ascend/tools/blob/master/ptdbg_ascend/README.md)》。

## PyTorch精度比对总体流程

1. 准备CPU或GPU训练工程。

2. 在环境下安装ptdbg_ascend工具。

3. 在训练脚本内插入ptdbg_ascend工具dump接口。

4. 执行训练dump数据。

5. 将CPU或GPU训练工程迁移为NPU训练工程。

   请参见《[PyTorch模型迁移和训练指南](https://www.hiascend.com/document/detail/zh/canncommercial/63RC1/modeldevpt/ptmigr/ptmigr_0001.html)》。

6. 在NPU环境下安装ptdbg_ascend工具。

7. 在NPU训练脚本内插入ptdbg_ascend工具dump接口。

8. NPU环境下执行训练dump数据。

9. 创建并配置精度比对脚本，例如compare.py。

10. 执行CPU或GPU dump与NPU dump数据的精度比对。

11. 比对结果分析。

## CPU或GPU及NPU精度数据dump 

### 接口说明

| 函数                | 描述                                                                                                | 是否必选                                                                                            |
| ------------------- |---------------------------------------------------------------------------------------------------| ------------------- |
| seed_all | 固定随机数。通过固定随机数保证模型的输入一致。在训练主函数开始前调用，避免随机数固定不全。取值为：<br/>* 第一个参数为随机数种子，参数示例：seed=1234，默认值为：1234。可选。<br/>* 第二个参数为确定性计算模式，参数示例：model=True，可配置True或False。默认为False。可选。 | 是 |
| set_dump_path       | 设置dump数据目录。在训练主函数开始前调用。取值为：<br/>* 第一个参数设置dump数据的.pkl文件名和路径，参数示例：'./dump_path/npu_dump.pkl'。必选。<br/>* 第一个参数设置dump数据目录名称。参数示例：dump_tag='dump_conv2d'，默认dump数据目录命名为ptdbg_dump_{version}。可选。<br/>{version}为当前安装ptdbg_ascend工具版本。目录结构参见“**dump数据存盘说明**”。 | 是                                           |
| register_hook | 注册dump回调函数。在set_dump_path之后调用。取值为：<br/>* 第一个参数为model对象。必选。<br/>* 第二个参数为注册工具的dump或溢出检测钩子，配置为acc_cmp_dump或overflow_check。二选一。<br/>* 第三个参数控制溢出次数，表示第N次溢出时，停止训练，过程中检测到溢出API对应ACL数据均dump。参数示例：overflow_nums=3。第二个参数配置overflow_check时可配置，默认不配置，即检测到1次溢出，训练停止。可选。<br/>* 第四个参数控制针对溢出API的dump模式。可取值"api"或"acl"，配置acl时表示dump ACL级别的溢出数据，此时set_dump_path参数不生效，dump数据目录由dump_config的.json文件配置，参数示例：dump_mode="acl"。默认不配置，即dump API级别的溢出数据。可选。<br/>* 第五个参数为acl dump的配置文件。dump_mode="acl"时，该参数必选；dump_mode="api"时，该参数不选。参数示例：dump_config='./dump.json'。<br/> * 第六个参数控制dump数据保存的rank目录。参数示例：rank=1。默认不配置，即自动读取dump数据所属的卡并保存在该卡对应的rank目录下。可选。目录结构参见“dump数据存盘说明”。 | 是 |
| set_dump_switch     | 设置dump范围。取值为：<br/>* 第一个参数为dump开关。可取值"ON"或者"OFF"。必选。须在选定dump开始的位置配置set_dump_switch("ON")；dump结束的位置设置set_dump_switch("OFF")，不设置OFF则表示dump从register_hook开始的所有数据。<br/>* 第二个参数为dump模式。可取值"list"、"range"、"stack"、"acl"、"api_list"、"api_stack"。参数示例：mode="list"。默认为空。可选。<br/>* 第三个参数为dump范围。可取值scope或api_list，根据model配置的模式选择dump的API范围。参数示例：scope=["Tensor_permute_1_forward", "Tensor_transpose_2_forward"])、api_list=["relu"]默认为空。可选。<br/>* 第四个参数为开启dump bool和整型的tensor以及浮点、bool和整型的标量。可取值"ON"或者"OFF"。参数示例：filter_switch="OFF"。默认不配置，即filter_switch="ON"，表示不dump上述数据。可选。 | 是                       |
| set_overflow_check_switch | 设置溢出检测范围。默认不配置该函数，全量进行溢出检测。<br/>* 第一个参数为检测开关。可取值"ON"或者"OFF"。如果只在特定的step溢出检测，则在期望溢出检测的step位置开始前插入set_overflow_check_switch("ON")，在step结束的位置插入set_overflow_check_switch("OFF")。必选。<br/>* 第二个参数为开启dump bool和整型的tensor以及浮点、bool和整型的标量。可取值"ON"或者"OFF"。参数示例：filter_switch="OFF"。默认不配置，即filter_switch="ON"，表示不dump上述数据。可选。 | 否 |
| set_backward_input        | 设置反向ACL级别dump时需要的反向输入的.npy文件。参数示例："acl_dump_xxx/Functional_conv2d_1_backward_input.0.npy"。该输入文件为首次运行训练dump得到的反向输入。 | 否                                                                       |

注：

- 以上函数使用及参数配置示例请参见“**dump函数使用说明**”。
- ptdbg_ascend工具默认情况下仅dump PyTorch模型的API输入输出数据进行精度比对，若在比对结果中发现某个API下可能存在ACL的精度问题，那么可以选择dump该API的ACL级别数据进行精度分析。
- 某些torch api的输出不是Tensor类型的数据，如torch.sort。对于此类API的反向过程进行ACL dump，工具会在运行日志中给出对应的Warning提示。如若想要进行该类API反向ACL dump，可以通过手动构建单API用例的方式进行ACL dump，具体用例可参考“**反向ACL dump用例说明**”。

### dump接口使用

#### 约束
- 环境约束：独占环境，CPU核心数192，固态硬盘（IO速度参考：固态硬盘 > 500MB/s，机械硬盘60 ~ 170MB/s）。

  dump数据量较小时（小于5G），参考dump速度0.1GB/s；dump数据量较大时，参考dump速度0.2GB/s。

- 进行CPU数据dump时，请安装torch包而非torch_npu包，避免工具无法识别使用场景，导致失败。

- TASK_QUEUE_ENABLE环境变量会导致API下发和执行异步进行，因此在ACL dump前需要将TASK_QUEUE_ENABLE关闭，即export TASK_QUEUE_ENABLE=0。

#### seed_all

seed_all函数的第一个参数为随机数种子，取默认值即可，无须配置；第二个参数默认关闭，不开启确定性计算时也无须配置。

- 示例1：仅固定随机数，不开启确定性计算

  ```bash
  seed_all()
  ```

- 示例2：固定随机数，开启确定性计算

  ```bash
  seed_all(mode=True)
  ```

  即使在相同的硬件和输入下，API多次执行的结果也可能不同，开启确定性计算是为了保证在相同的硬件和输入下，API多次执行的结果相同。

  确定性计算会导致API执行性能降低，建议在发现模型多次执行结果不同的情况下开启。

seed_all函数可固定随机数的范围如下表。

| API                                      | 固定随机数                  |
| ---------------------------------------- | --------------------------- |
| os.environ['PYTHONHASHSEED'] = str(seed) | 禁止Python中的hash随机化    |
| random.seed(seed)                        | 设置random随机生成器的种子  |
| np.random.seed(seed)                     | 设置numpy中随机生成器的种子 |
| torch.manual_seed(seed)                  | 设置当前CPU的随机种子       |
| torch.cuda.manual_seed(seed)             | 设置当前GPU的随机种子       |
| torch.cuda.manual_seed_all(seed)         | 设置所有GPU的随机种子       |
| torch_npu.npu.manual_seed(seed)          | 设置当前NPU的随机种子       |
| torch_npu.npu.manual_seed_all(seed)      | 设置所有NPU的随机种子       |
| torch.backends.cudnn.enable=False        | 关闭cuDNN                   |
| torch.backends.cudnn.benchmark=False     | cuDNN确定性地选择算法       |
| torch.backends.cudnn.deterministic=True  | cuDNN仅使用确定性的卷积算法 |

需要保证CPU或GPU以及NPU的模型输入完全一致，dump数据的比对才有意义，seed_all并不能保证模型输入完全一致，如下表所示场景需要用户自行保证输入的一致性。

| 场景            | 固定方法           |
| --------------- | ------------------ |
| 数据集的shuffle | 将shuffle设置为off |
| dropout         | 将dropout设置为off |

#### set_dump_path

- 示例1：设置dump数据的.pkl文件名和路径

  ```bash
  set_dump_path('./dump_path/npu_dump.pkl')
  ```

  .pkl文件名需要用户自定义命名，该示例下的dump数据会在./dump_path路径下生成ptdbg_dump_{version}目录，pkl文件以及dump数据均保存在该目录下。

- 示例2：设置dump数据目录名称

  ```bash
  set_dump_path('./dump_path/myDump.pkl', dump_tag='dump_conv2d')
  ```

  该示例下会将生成的`ptdbg_dump_{version}`目录名称变更为`dump_conv2d_{version}`。

**说明**：若以相同的.pkl文件名和dump_tag运行两次，则会因同名导致覆盖。

#### register_hook

- 示例1：注册dump回调函数

  ```bash
  register_hook(model, acc_cmp_dump)
  ```

- 示例2：dump指定API的ACL级别数据

  ```bash
  register_hook(model, acc_cmp_dump, dump_mode='acl', dump_config='./dump.json')
  ```

  需要配置set_dump_switch的mode="acl"以及scope指定为前向或反向API，请参见“set_dump_switch”的示例。

  该场景set_dump_path不生效，由dump_config中的dump.json文件配置dump数据目录。

- 示例3：溢出检测dump

  ```bash
  register_hook(model, overflow_check, overflow_nums=3)
  ```

  该场景set_dump_path不生效，dump执行时会在当前目录自动生成ptdbg_dump_{version}目录，保存溢出数据。

  单机多卡场景时，需要检测到至少有一张卡溢出次数达到overflow_nums时，训练结束。

  仅支持NPU环境。

- 示例4：dump指定API的ACL级别溢出数据

  ```bash
  register_hook(model, overflow_check, dump_mode='acl', dump_config='./dump.json')
  ```

  该场景set_dump_path不生效，由dump_config中的dump.json文件配置溢出数据目录。

  仅支持NPU环境。

- 示例5：指定dump数据保存的rank目录

  ```bash
  register_hook(model, acc_cmp_dump, rank=1)
  ```

  当rank_id未知时，可参见“**rank_id获取示例**”。

#### set_dump_switch

set_dump_switch可配置多中dump模式，示例如下：

- 示例1：dump指定API列表

  ```bash
  set_dump_switch("ON", mode="list", scope=["Tensor_permute_1_forward", "Tensor_transpose_2_forward", "Torch_relu_3_backward"])
  ```

- 示例2：dump指定范围

  ```bash
  set_dump_switch("ON", mode="range", scope=["Tensor_abs_1_forward", "Tensor_transpose_3_forward"])
  ```

- 示例3：STACK模式，只dump堆栈信息

  ```bash
  set_dump_switch("ON", mode="stack", scope=["Tensor_abs_1_forward", "Tensor_transpose_3_forward"])
  ```

- 示例4：dump指定前向API的ACL级别数据

  ```bash
  register_hook(model, acc_cmp_dump, dump_mode='acl', dump_config='./dump.json')
  set_dump_switch("ON", mode="acl", scope=["Tensor_permute_1_forward"])
  ```

  需要配置register_hook的dump_mode='acl'和dump_config配置文件。

- 示例4：dump指定反向API的ACL级别数据

  ```bash
  register_hook(model, acc_cmp_dump, dump_mode='acl', dump_config='./dump.json')
  set_dump_switch("ON", mode="acl", scope=["Functional_conv2d_1_backward"])
  set_backward_input(["acl_dump_xxx//Functional_conv2d_1_backward_input.0.npy"])
  ```

  需要配置register_hook的dump_mode='acl'和dump_config配置文件，并通过set_backward_input设置反向API输入的.npy文件。

- 示例5：dump指定某一类API的API级别输入输出数据

  ```bash
  set_dump_switch("ON", mode="api_list", api_list=["relu"])
  ```

  mode="api_list"时不配置scope。

- 示例6：dump全部API级别输入输出数据以及相应堆栈信息

  ```bash
  set_dump_switch("ON", mode="api_stack")
  ```

  mode="api_stack"时不配置scope。

- 示例7： dump全部API级别输入输出数据并包含bool和整型的tensor以及浮点、bool和整型的标量，默认不配置为ON，会过滤bool和整型数据

  ```bash
  set_dump_switch("ON", filter_switch="OFF")
  ```

  配置filter_switch="OFF"同时也可以配置mode、scope和api_list，除dump ACL级别数据。

以上示例默认情况下不配置set_dump_switch("OFF")，表示从set_dump_switch("ON")插入的位置开始到整体训练结束均进行示例中配置的范围dump；若在脚本中插入set_dump_switch("OFF")，则dump操作在此结束。

#### set_overflow_check_switch

- 示例1：指定范围溢出检测

  ```bash
  register_hook(model, overflow_check)
  set_overflow_check_switch("ON")
  
  ...
  
  set_overflow_check_switch("OFF")
  ```

  该场景set_dump_path不生效，dump执行时会在当前目录自动生成ptdbg_dump_{version}目录，保存溢出数据。

- 示例2：前向API的ACL级别范围溢出检测

  ```bash
  register_hook(model, overflow_check, dump_mode='acl', dump_config='./dump.json')
  set_overflow_check_switch("ON")
  
  ...
  
  set_overflow_check_switch("OFF")
  ```

  该场景set_dump_path不生效，由dump_config中的dump.json文件配置溢出数据目录。

仅支持NPU环境。

#### set_backward_input 

函数示例请参见“set_dump_switch”的“dump指定反向API的ACL级别数据”示例。

#### dump.json配置文件说明

**dump.json配置示例**

```bash
{
 "dump":
 {
         "dump_list":[],
         "dump_path":"./dump/output",
         "dump_mode":"all",
         "dump_op_switch":"on"
 }
}
```

**dump.json参数说明**

| 字段名         | 说明                                                         |
| -------------- | ------------------------------------------------------------ |
| dump_list      | 待dump数据的API模型。为空，无需配置。                        |
| dump_path      | dump数据文件存储到运行环境的目录，支持配置绝对路径或相对路径。 |
| dump_mode      | dump数据模式，配置如下： * output：dump API的输出数据。默认值。 * input：dump API的输入数据。 *  all：dump API的输入、输出数据。 |
| dump_op_switch | 单API模型dump数据开关，配置如下： * off：关闭单API模型dump，默认值。 * on：开启单API模型dump。 |

**dump目录说明**

配置register_hook的dump_config后，采集的dump数据会在{dump_path}/{time}/{deviceid}/{model_id}目录下生成，例如“/home/HwHiAiUser/output/20200808163566/0/0”

```bash
├── 20230131172437
│   └── 1
│       ├── 0
│       │   ├── Add.Add.45.0.1675157077183551
│       │   ├── Cast.trans_Cast_0.31.0.1675157077159449
│       │   ├── Cast.trans_Cast_5.43.0.1675157077180129
│       │   ├── MatMul.MatMul.39.0.1675157077172961
│       │   ├── Mul.Mul.29.0.1675157077155731
│       │   ├── NPUAllocFloatStatus.NPUAllocFloatStatus.24.0.1675157077145262
│       │   ├── TransData.trans_TransData_1.33.0.1675157077162791
│       │   └── TransData.trans_TransData_4.41.0.1675157077176648
│       ├── 1701737061
│       │   └── Cast.trans_Cast_2.35.0.1675157077166214
│       ├── 25
│       │   └── NPUClearFloatStatus.NPUClearFloatStatus.26.0.1675157077150342
│       └── 68
│           └── TransData.trans_TransData_3.37.0.1675157077169473
```

### dump数据存盘说明

dump结果目录结构示例如下：

```bash
├── dump_path
│   └── ptdbg_dump_{version}
│       ├── rank0
│       │   ├── myDump
|       |   |    ├── Tensor_permute_1_forward.npy
|       |   |    ...
|       |   |    └── Fcuntion_linear_5_backward_output.npy
│       │   └── myDump.pkl
│       ├── rank1
|       |   ├── myDump
|       |   |   └── ...
|       |   └── myDump.pkl 
│       ├── ...
│       |
|       └── rank7
```

其中ptdbg_dump_{version}为未设置set_dump_path的dump_tag参数时的默认命名；rank为设备上各卡的ID，每张卡上dump的数据会生成对应dump目录。

**精度比对dump场景**

假设set_dump_path配置的dump文件名为npu_dump.pkl，此时dump的结果为两部分：

* npu_dump.pkl文件：包含dump数据的API名称、dtype、 shape以及各数据的max、min、mean统计信息。

* npu_dump_timestamp目录：目录下为npy格式的dump数据。

   npy文件保存的前缀和PyTorch对应关系如下

   | 前缀       | Torch模块           |
   | ---------- | ------------------- |
   | Tensor     | torch.Tensor        |
   | Torch      | torch               |
   | Functional | torch.nn.functional |
   | NPU        | NPU亲和算子         |
   | VF         | torch._VF           |

**api_stack dump场景**

当set_dump_switch配置mode="api_stack" 时，dump结果的文件名会添加api_stack前缀，dump结果如下：

* api_stack_npu_dump.pkl
* api_stack_npu_dump_timestamp目录

api_stack为堆栈信息。

**溢出检测dump场景**

register_hook设置了overflow_check时，检测API溢出，此时不需要配置set_dump_path，dump执行时会在当前目录自动生成ptdbg_dump_v1.0目录，dump结果如下：

* Overflow_info_{timestamp}.pkl
* Overflow_info_{timestamp}目录

## CPU或GPU与NPU精度数据比对

### 接口说明

| 函数                | 描述                                                         |
| ------------------- | ------------------------------------------------------------ |
| compare_distributed | 将CPU或GPU与NPU的dump文件进行比对，支持单机单卡和单机多卡，可同时比对多卡的dump数据。可自动检索和匹配对应卡和进程所dump的数据文件，再调用compare进行比对。单机单卡时与compare二选一。取值为：<br/>* 第一个参数配置NPU环境下的dump目录npu_dump_dir，即set_dump_path函数的dump_tag参数对应的目录名称。参数示例：'npu_dump/dump_conv2d_v1.0'。必选。<br/>* 第二个参数配置CPU、GPU或NPU环境下的dump目录bench_dump_dir，即set_dump_path函数的dump_tag参数对应的目录名称。参数示例：'gpu_dump/dump_conv2d_v1.0'。必选。<br/>* 第三个参数配置比对结果输出目录output_path。参数示例：'./output'。文件名称基于时间戳自动生成，格式为：`compare_result_{timestamp}_rank{npu_ID}-rank{cpu/gpu/npu_ID}.csv`。必选。<br/>* 第四个参数开始支持compare的所有可选参数。 |
| compare             | 将CPU或GPU与NPU的dump文件进行比对，仅支持单机单卡。取值为：<br/>* 第一个参数配置dump数据目录，须指定已存在的目录。必选。<br/>* 第二个参数配置存放比对结果的目录output_path。参数示例：'./output'。文件名称基于时间戳自动生成，格式为：`compare_result_{timestamp}.csv`。必选。<br/>* 第三个参数配置stack_mode的开关。仅当dump数据时配置set_dump_switch的mode="api_stack"时需要开启。参数示例：stack_mode=True，默认为False。可选。<br/>* 第四个参数配置auto_analyze，用于开启自动精度分析，开启后工具自动针对比对结果进行分析，识别到第一个精度不达标节点，在比对结果文件中的“Accuracy Reached or Not”列显示为No，并给出问题可能产生的原因。参数示例：auto_analyze=False，默认为True。可选。<br/>* 第五个参数配置suffix，用于标识比对结果的文件名。配置的suffix值在比对结果文件名的compare_result和{timestamp}中间插入，例如：compare_result_suffix_{timestamp}。默认为空。可选。 |
| parse               | 提取dump信息中的堆栈信息及数据统计信息。取值为：<br/>* 第一个参数指定dump数据文件中的pkl文件名，指定为配置set_dump_switch的mode="api_stack"或mode="stack"时的dump pkl文件。参数示例："./npu_dump.pkl"。必选。<br/>* 第二个参数指定待提取的API接口前缀。参数示例："Torch_norm_1_forward"。必选。<br/>仅NPU环境支持。 |

注：以上函数均通过单独创建精度比对脚本执行，请根据实际场景选择使用，使用及参数配置示例请参见“**比对函数使用说明**”。

### 比对接口使用

#### 约束

- 环境约束：独占环境，CPU核心数192，固态硬盘（IO速度参考：固态硬盘 > 500MB/s，机械硬盘60~170MB/s）。

  比对数据量较小时（单份文件小于10GB），参考比对速度0.1GB/s；比对数据量较大时，参考比对速度0.3GB/s。

- NPU自研API，在CPU或GPU若没有对应的API，该API的dump数据不比对。

- NPU与CPU或GPU的计算结果误差可能会随着模型的执行不断累积，最终会出现同一个API因为输入的数据差异较大而无法比对的情况。

- CPU或GPU与NPU中两个相同的API会因为调用次数不同导致无法比对或比对到错误的API，不影响整体运行，忽略该API。

#### compare_distributed

创建比对脚本，例如compare_distributed.py，拷贝如下代码，具体参数请根据实际环境修改。

```bash
from ptdbg_ascend import *
compare_distributed('npu_dump/dump_conv2d_v1.0', 'gpu_dump/dump_conv2d_v1.0', './output')
```

#### compare

单机单卡场景下创建比对脚本，例如compare.py，拷贝如下代码，具体参数请根据实际环境修改。

```bash
from ptdbg_ascend import *
dump_result_param={
"npu_pkl_path": "./api_stack_npu_dump.pkl",
"bench_pkl_path": "./api_stack_gpu_dump.pkl",
"npu_dump_data_dir": "./api_stack_npu_dump_20230104_13434",
"bench_dump_data_dir": "./api_stack_gpu_dump_20230104_132544",
"is_print_compare_log": True
}
compare(dump_result_param, "./output", shape_flag=True, stack_mode=True)
```

#### parse

创建堆栈信息及数据统计信息提取脚本，例如parse.py，拷贝如下代码，具体参数请根据实际环境修改。

```bash
from ptdbg_ascend import *
parse("./npu_dump.pkl", "Torch_batch_normal_1_forward")
```



### 计算精度评价指标

PyTorch精度比对是以CPU或GPU的计算结果为标杆，计算Cosine（余弦相似度）和MaxAbsError（最大绝对误差），根据这两个结果判断API在运行时是否存在精度问题。

计算精度评价指标：

1. Cosine：通过计算两个向量的余弦值来判断其相似度，数值越接近于1说明计算出的两个张量越相似，实际可接受阈值为大于0.99。在计算中可能会存在nan，主要由于可能会出现其中一个向量为0。
2. MaxAbsError：当最大绝对误差越接近0表示其计算的误差越小，实际可接受阈值为小于0.001。

精度比对结果csv文件中只需要通过Accuracy Reached or Not来判断计算精度是否达标，判断标准如下：

1. Cosine < 0.99 且 MaxAbsError > 0.001时，记为精度不达标，标记为“No”。
2. 其余情况下记为精度达标，标记为“Yes”。


## 场景化示例
### 单机单卡场景精度比对
**精度分析建议**

PyTorch训练场景的精度问题分析建议参考以下思路进行精度比对和比对结果分析：

1. 整网比对：dump整网数据并进行精度比对，初步定位异常范围。
2. 缩小范围：根据Accuracy Reached or Not找出不符合精度标准的API。
3. 范围比对：对不符合精度标准的API重新dump。
4. 分析原因并优化：分析API精度不符合标准的原因并进行优化调整。
5. 整网比对：重新进行整网比对，判断优化后的API是否已符合精度标准以及是否出现新的精度问题。
6. 重复1~5步，直到不存在精度问题为止。

**精度分析示例**

1. dump整网数据。

   分别dump CPU或GPU以及NPU数据，在PyTorch训练脚本插入dump接口，示例代码如下（下面以NPU为例，CPU或GPU dump基本相同）：

   ```bash
   from ptdbg_ascend import *
   
   # 在main函数开始前固定随机数
   seed_all()
   
   # 配置dump数据的.pkl文件名和路径
   set_dump_path("./npu_dump.pkl", dump_tag='all')
   
   # 注册dump回调函数
   register_hook(model, acc_cmp_dump)
   
   ...
   
   # 在第一个迭代开始的位置开启dump且不配置dump关闭，即dump整网数据，另外dump堆栈信息
   set_dump_switch("ON", mode="api_stack")
   ```

2. 比对整网数据。

   第1步中的NPU dump数据文件为npu_dump.pkl，假设NPU dump npy数据目录为npu_dump_20230104_13434，GPU dump数据文件为gpu_dump.pkl，GPU dump npy数据目录为gpu_dump_20230104_132544。

   创建并配置精度比对脚本，以创建compare.py为例，示例代码如下：

   ```bash
   from ptdbg_ascend import *
   dump_result_param={
   "npu_pkl_path": "./api_stack_npu_dump.pkl",
   "bench_pkl_path": "./api_stack_gpu_dump.pkl",
   "npu_dump_data_dir": "./api_stack_npu_dump_20230104_13434",
   "bench_dump_data_dir": "./api_stack_gpu_dump_20230104_132544",
   "is_print_compare_log": True
   }
   compare(dump_result_param, "./output", True, stack_mode=True)
   ```

   执行比对：

   ```bash
   python3 compare.py
   ```

   

3. 找出存在问题的API。

   根据第2步结果文件，找出中的Accuracy Reached or No字段显示为NO的API，针对该API执行后续比对操作，分析该API存在的精度问题。

4. （可选）提取指定API的堆栈信息和dump数据统计信息。

   通过parse接口可以清晰的显示特定API的堆栈信息和dump数据统计信息，结合堆栈信息分析代码中可能存在的精度问题。

   创建并配置提取脚本，以创建parse.py为例，示例代码如下：

   ```bash
   from ptdbg_ascend import *
   
   # 提取dump信息中第1次调用的API：Torch_batch_normal的堆栈信息及数据统计信息
   parse("./npu_dump.pkl", "Torch_batch_normal_1_forward")
   ```

   执行提取：

   ```bash
   python3 parse.py
   ```

   

5. （可选）指定API dump数据。

   - dump指定前向API的ACL级别数据

     ```bash
     from ptdbg_ascend import *
     
     # 固定随机数，开启确定性计算
     seed_all(mode=True)
     set_dump_path("./npu_dump1.pkl", dump_tag='forward')
     register_hook(model, acc_cmp_dump, dump_mode='acl', dump_config='./dump.json')
     
     # dump指定前向API的ACL级别数据、bool和整型的tensor以及浮点、bool和整型的标量
     set_dump_switch("ON", mode="acl", scope=["Tensor_permute_1_forward"], filter_switch="OFF")
     
     ...
     
     set_dump_switch("OFF")
     ```

   - dump指定反向API的ACL级别数据

     ```bash
     from ptdbg_ascend import *
     
     # 固定随机数，开启确定性计算
     seed_all(mode=True)
     set_dump_path("./npu_dump1.pkl", dump_tag='backward')
     register_hook(model, acc_cmp_dump, dump_mode='acl', dump_config='./dump.json')
     
     # dump指定反向API的ACL级别数据、bool和整型的tensor以及浮点、bool和整型的标量
     set_dump_switch("ON", mode="acl", scope=["Functional_conv2d_1_backward"], filter_switch="OFF")
     set_backward_input(["acl_dump_xxx//Functional_conv2d_1_backward_input.0.npy"])
     
     ...
     
     set_dump_switch("OFF")
     ```

6. （可选）重新比对。

   根据第5步的dump数据重新配置compare.py并执行比对，可以对单API模型进行问题复现。

**注意事项**

* dump_mode="acl"场景下，会增加npu的内存消耗，请谨慎开启。
* 部分API存在调用嵌套关系，比如functional.batch_norm实际调用torch.batch_norm，该场景会影响acl init初始化多次，导致功能异常。

### 单机多卡场景精度比对
精度工具单机多卡场景下的精度比对步骤与单机单卡场景完全一致，请参见“**单机单卡场景精度比对**”章节。唯一不同的是精度比对时需要使用“compare_distributed”函数进行比对，如下示例：

假设NPU dump npy数据目录为npu_dump/dump_conv2d_v1.0，GPU dump npy数据目录为gpu_dump/dump_conv2d_v1.0。

1. 创建比对脚本，例如compare_distributed.py，拷贝如下代码。

   ```bash
   from ptdbg_ascend import *
   compare_distributed('npu_dump/dump_conv2d_v1.0', 'gpu_dump/dump_conv2d_v1.0', './output')
   ```

2. 执行比对：

   ```bash
   python3 compare_distributed.py
   ```

两次运行须用相同数量的卡，传入`compare_distributed`的两个文件夹下须有相同个数的rank文件夹，且不包含其他无关文件，否则将无法比对。

**单机多卡set_dump_path注意事项**

单机多卡一般为多进程，须保证每个进程都正确调用set_dump_path，或把set_dump_path插入到import语句后，如：

```bash
from ptdbg_ascend import *
seed_all()
set_dump_path('./dump_resnet/myDump.pkl')
```

如此可保证set_dump_path在每个进程都被调用。

**单机多卡register_hook注意事项**

register_hook需要在set_dump_path之后调用，也需要在每个进程上被调用，建议在搬运模型数据到卡之后调用。识别方法如下：

- 找到训练代码中遍历epoch的for循环或遍历数据集的for循环，把register_hook放到循环开始前即可。
- 找到训练代码中调用DDP或者DistributedDataParallel的代码行，把register_hook放到该代码行所在的代码块之后。
- 若代码中均无以上两种情况，需要保证register_hook在模型定义之后插入，并配置rank参数。rank参数获取rank_id请参见“**rank_id获取示例**”。

### NPU vs NPU精度比对
对于NPU vs NPU场景，是针对同一模型，进行迭代（模型、API版本升级或设备硬件升级）时存在的精度下降问题，对比相同模型在迭代前后版本的API计算数值，进行问题定位。

一般情况下迭代涉及NPU自定义算子，因此，可以仅dump NPU自定义算子进行比对。比对精度问题分析请参见“PyTorch训练场景的精度问题分析”章节。

工具当前支持dump NPU自定义算子如下：

| NPU自定义算子 |
| ------ | 
| torch_npu.one_ | 
| torch_npu.npu_sort_v2 | 
| torch_npu.npu_transpose |
| torch_npu.npu_broadcast |
| torch_npu.npu_dtype_cast |
| torch_npu.empty_with_format |
| torch_npu.npu_one_hot |
| torch_npu.npu_stride_add |
| torch_npu.npu_ps_roi_pooling |
| torch_npu.npu_roi_align |
| torch_npu.npu_nms_v4 |
| torch_npu.npu_iou |
| torch_npu.npu_nms_with_mask |
| torch_npu.npu_pad |
| torch_npu.npu_bounding_box_encode |
| torch_npu.npu_bounding_box_decode |
| torch_npu.npu_batch_nms |
| torch_npu.npu_slice |
| torch_npu._npu_dropout |
| torch_npu.npu_indexing
| torch_npu.npu_ifmr |
| torch_npu.npu_max |
| torch_npu.npu_scatter |
| torch_npu.npu_layer_norm_eval |
| torch_npu.npu_alloc_float_status |
| torch_npu.npu_get_float_status |
| torch_npu.npu_clear_float_status |
| torch_npu.npu_confusion_transpose |
| torch_npu.npu_bmmV2 |
| torch_npu.fast_gelu |
| torch_npu.npu_sub_sample |
| torch_npu.npu_deformable_conv2d |
| torch_npu.npu_mish |
| torch_npu.npu_anchor_response_flags |
| torch_npu.npu_yolo_boxes_encode |
| torch_npu.npu_grid_assign_positive |
| torch_npu.npu_normalize_batch |
| torch_npu.npu_masked_fill_range |
| torch_npu.npu_linear |
| torch_npu.npu_bert_apply_adam |
| torch_npu.npu_giou |
| torch_npu.npu_ciou |
| torch_npu.npu_ciou_backward |
| torch_npu.npu_diou |
| torch_npu.npu_diou_backward |
| torch_npu.npu_sign_bits_pack |
| torch_npu.npu_sign_bits_unpack |

### 溢出检测场景

溢出检测是针对NPU的PyTorchAPI，检测是否存在溢出的情况。当前仅支持识别aicore浮点溢出。

溢出检测原理：针对溢出阶段，开启acl dump模式，重新对溢出阶段执行，落盘数据。

建议按照如下步骤操作：

1. 准备CPU或GPU训练工程。

2. 将CPU或GPU训练工程迁移为NPU训练工程。

   请参见《[PyTorch模型迁移和训练指南](https://www.hiascend.com/document/detail/zh/canncommercial/63RC1/modeldevpt/ptmigr/ptmigr_0001.html)》。

3. 在NPU环境下安装ptdbg_ascend工具。

4. 在NPU训练脚本内插入ptdbg_ascend工具溢出检测接口。

   - 示例1：全量溢出检测

     ```bash
     from ptdbg_ascend import *
     seed_all()
     ...
     # 设置检测到3次溢出后退出训练
     register_hook(model, overflow_check, overflow_nums=3)
     
     ...
     ```

     单机多卡使用时各卡单独计算溢出次数。

   - 示例2：dump指定API的ACL级别溢出数据

     ```bash
     from ptdbg_ascend import *
     seed_all()
     ...
     # dump指定API的ACL级别溢出数据
     register_hook(model, overflow_check, dump_mode='acl', dump_config='./dump.json')
     
     # 在期望溢出检测的step位置开始前打开溢出检测开关
     set_overflow_check_switch("ON")
     
     ...
     
     # 在step结束的位置关闭溢出检测开关
     set_overflow_check_switch("OFF")
     
     ...
     ```

   - 示例3：dump指定反向API的ACL级别的溢出数据

     1. 进行全量溢出检测

        ```bash
        from ptdbg_ascend import *
        seed_all()
        ...
        # 设置检测到3次溢出后退出训练
        register_hook(model, overflow_check)
        
        ...
        ```

     2. dump指定反向API的ACL级别的溢出数据

        ```bash
        from ptdbg_ascend import *
        seed_all()
        ...
        # dump指定反向API的ACL级别溢出数据
        register_hook(model, acc_cmp_dump, dump_mode='acl', dump_config='./dump.json')
        set_dump_switch("ON", mode="acl", scope=["Functional_conv2d_1_backward"])
        set_backward_input(["acl_dump_xxx//Functional_conv2d_1_backward_input.0.npy"])
        ```

   针对前向溢出API，可以通过overflow_nums，配置允许的溢出次数，并将每次溢出API的全部ACL数据dump下来，到达指定溢出次数后停止，停止后会看到堆栈打印包含如下字段。

   ```bash
   ValueError: [overflow xxx times]: dump file is saved in 'xxxxx.pkl'.
   ```

   其中xxx times为用户设置的次数，xxxxx.pkl为文件生成路径。

5. NPU环境下执行训练dump溢出数据。

**注意事项**

* dump_mode="acl"场景下，会增加npu的内存消耗，请谨慎开启。
* 部分API存在调用嵌套关系，比如functional.batch_norm实际调用torch.batch_norm，该场景会影响acl init初始化多次，导致功能异常。

## 附录

### 反向ACL dump用例说明

当前昇腾AI处理器上的PyTorch框架通过torch_npu.npu中的init_dump(),set_dump()和finalize_dump()接口来进行ACL级别的数据采集。首先init_dump()会进行初始化dump配置，然后通过set_dump()接口传入配置文件来配置dump参数，最后通过finalize_dump来结束dump。 下面将以torch.sort运算的反向过程为例，介绍反向ACL数据dump的方法。

```bash
import numpy as np
import torch
import torch_npu
torch_npu.npu.set_device("npu:0")
input = torch.tensor(np.load(input_path)).requires_grad_().npu()
grad = torch.tensor(np.load(grad_path)).requires_grad_().npu()
b, c = torch.sort(input)
torch_npu.npu.init_dump()
torch_npu.npu.set_dump("dump.json")
torch_npu.npu.synchronize()
b.backward(grad)
torch_npu.npu.synchronize()
torch_npu.npu.finalize_dump()
```

- input_path是该API前向运算的输入，可以通过ACL dump的API名称获得。如想要对Torch_sort_0_backward进行ACL dump，则该反向API对应的前向过程输入为Torch_sort_0_forward_input.0.npy。
- grad_path是该API反向运算的输入，同理可以通过期望dump的API名称获得。

- b, c是torch.sort的输出，分别表示排序后的tensor和排序后tensor中的各元素在原始tensor中的位置。对torch.sort进行反向时，需要对b进行backward。

**dump.json配置**

```bash
{
  "dump":
  {
    "dump_list": [],
    "dump_path": "/home/HwHiAiUser/dump/output",
    "dump_mode": "all",
    "dump_op_switch": "on"
  }
}
```

**查看dump数据**

采集的dump数据会在{dump_path}/{time}/{device_id}/{model_id}/{data_index}目录下生成。

### rank_id获取示例

**通过环境变量获取**

当前进程的rank_id可能保存在环境变量中，比如`LOCAL_RANK`。则可以通过如下示例来检查当前进程的rank_id：

```bash
import os
print("Local rank is: ", os.environ.get('LOCAL_RANK'))
```

若打印结果显示该环境变量被配置过，如：

```bash
# 以单机8卡为例
Local rank is: 0
Local rank is: 2
Local rank is: 3
Local rank is: 1
Local rank is: 4
Local rank is: 5
Local rank is: 6
Local rank is: 7
```

那么将该环境变量作为rank传参即可自动获取到rank_id，如

```bash
register_hook(model, acc_cmp_dump, rank=os.environ.get('LOCAL_RANK')
```

**通过命令行参数获取**

若通过命令行参数传入rank_id，比如`--local_rank`。那么可以在代码中找到`args.local_rank` 来作为rank参数值。比如“

```bash
register_hook(model, acc_cmp_dump, rank=args.local_rank)
```

## FAQ

### 1. 单机多卡场景dump目录下只生成一个rank目录或pkl文件格式损坏

**故障现象**

dump目录下只生成一个rank目录或dump目录下的pkl文件格式损坏、内容不完整。 

**故障原因**

通常是因为register_hook没有正确配置，带着工具没有获取正确的`rank_id`（从rank参数读取或从模型参数的device_id读取）。

**故障处理**

register_hook需要在set_dump_path之后调用，也需要在每个进程上被调用，建议在搬运模型数据到卡之后调用。识别方法如下：

- 找到训练代码中遍历epoch的for循环或遍历数据集的for循环，把register_hook放到循环开始前即可。
- 找到训练代码中调用DDP或者DistributedDataParallel的代码行，把register_hook放到该代码行所在的代码块之后。
- 若代码中均无以上两种情况，那么尽可能把这行代码往后放，并配置register_hook的rank参数。

### 2. HCCL 报错： error code: EI0006

**故障现象**

使用ptdbg_ascend工具时，报错： error code: EI0006。

**故障原因**

CANN软件版本较低导致不兼容。

**故障处理**

升级新版CANN软件版本。
