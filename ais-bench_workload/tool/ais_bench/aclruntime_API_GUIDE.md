# aclruntime API使用指南
## API简介

AISBench通过在基于昇腾硬件的离线模型（.om模型）上运行推理功能，进行模型推理性能测试。用户可以通过[命令行](https://gitee.com/ascend/tools/tree/master/ais-bench_workload/tool/ais_bench#%E4%BD%BF%E7%94%A8%E6%96%B9%E6%B3%95)以及[Python API接口](https://gitee.com/ascend/tools/blob/master/ais-bench_workload/tool/ais_bench/API_GUIDE.md)执行模型的推理过程。这两种方法主要使用了Python端封装的类InferSession，调用其中封装的参数设置、模型推理函数，获得推理结果和性能测试情况。

aclruntime绑定了Python前端InferSession类和C++后端PyInferenceSession类，直接开放aclruntime推理API，使用户可以直接调用aclruntime模块，用Python函数代码直接调用模型推理后端的C++函数，减少了在Python端的一些操作，提升模型推理和开发的效率。

使用aclruntime API需要安装`aclruntime`包。安装方法参考[ais_bench推理工具使用指南](https://gitee.com/ascend/tools/blob/master/ais-bench_workload/tool/ais_bench/README.md)的“工具安装”章节。

## aclruntime API 基本流程

### 导入依赖包

使用aclruntime API前需要导入如下依赖：

```python
import aclruntime
```

### 加载模型
aclruntime.InferenceSession 是运行模型推理aclruntime API的主要类，用于加载om模型和执行om模型的推理，模型推理前需要初始化一个InferenceSession的实例。
```python
# session_options()构建了一个包含：log_level、loop、aclJsonPath的实例，存储模型信息
options = aclruntime.session_options()
# 该初始化表示在device_id的npu芯片上加载模型,并将options作为参数传入
session = aclruntime.InferenceSession(model_path, device_id, options)
```

### 数据迁移至device
aclruntime API直接调用后端C++函数，在该过程中需要数据在npu上，所以需要将模型的输入数据提前放在device侧。
```python
# ndata是一个Numpy数组，它的shape是模型输入所需的
# 利用aclruntime.Tensor将数据转化为适合模型推理的类型
tensor = aclruntime.Tensor(ndata)
# 将数据放至device上
tensor.to_device(device_id)
```

### 执行模型推理
建立好模型推理的实例session后，准备outnames，表示模型输出结果的名称。在npu芯片上的数据和配置都已经设定完成，调用session的成员函数接口进行模型推理，接口返回值就是推理结果。
```python
# outnames表示模型推理结果输出的名称
outnames = [meta.name for meta in session.get_outputs()]
# feeds数据类型是list，存放着一组模型推理的Tensor类型的数据。outputs是ndarray格式的tensor，表示模型推理的输出。
outputs = session.run(outnames, feeds)
```

### 获取模型数据性能
推理结束，推理的性能数据也保存在session中，可以通过session的接口获取性能数据。
```python
# exec_time_list 按先后顺序保留了所有session在执行推理的时间。
ssession.sumary().exec_time_list
```

## aclruntime API 详细介绍
### 通用函数说明

下面将介绍aclruntime API中通用的一些函数接口。这些函数接口会在各种场景中基本都会被使用，用来初始化模型、构造推理输入、配置参数等功能。

#### session_options函数

**使用示例**

```python
# 初始化包含日志等级log_level表示记录信息的程度，默认值是2；循环推理次数loop，表示模型推理需要循环的次数，默认值是1；
# 以及配置文件地址aclJsonPath，默认值为空字符串，可以后续设定。
options = aclruntime.session_options()
```

**功能说明**

创建一个包含日志等级（`log_level`）、循环推理次数（`loop`）和配置文件地址（`aclJsonPath`）的对象。

**参数说明**

无

**输出说明**

输出模型推理所需的数据信息的实例。


#### InferenceSession初始化实例函数

**使用示例**

```python
# 输入模型地址，推理npu的id，以及配置信息对象。
session = aclruntime.InferenceSession(model_path, device_id, options)
```

**功能说明**

使用`aclruntime.InferenceSession`调用，加载om模型，并后续用于运行模型推理。

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|model_path|string|模型存在路径|
|device_id|int|推理npu的ID|
|options|SessionOptions对象|存放模型推理所需的参数信息|

**输出说明**

用于模型推理的实例。


#### get_inputs函数

**使用示例**

```python
session = aclruntime.InferenceSession(model_path, device_id, options)
input_desc = session.get_inputs()
```

**功能说明**

使用推理实例调用。用于获取aclruntime.InferenceSession()加载的模型的输入节点信息。

**参数说明**

无

**输出说明**

输出list[aclruntime.TensorDesc]类型数据，包括输入数据的属性信息。
TensorDesc:C++后端侧代码，结构体。存放name、TensorDataType、输入的shapes、输入的size等类型信息。


#### get_outputs函数

**使用示例**

```python
session = aclruntime.InferenceSession(model_path, device_id, options)
output_desc = session.get_outputs()
```

**功能说明**

使用推理实例调用。用于获取aclruntime.InferenceSession()加载的模型的输出节点信息。

**参数说明**

无

**输出说明**

输出list[aclruntime.TensorDesc]类型数据，包括输出数据属性信息。


#### Tensor函数

**使用示例**

```python
# 创建一个shape形状的numpy数组
ndata = np.full(shape, 1).astype(np.float32)
# 使用numpy数组构建aclruntime的Tensor类对象
tensor = aclruntime.Tensor(ndata)
```

**功能说明**

使用aclruntime.Tensor()调用。根据ndarray类型的数据构造TensorBase类对象，存储模型推理所需的数据和信息。
TensorBase：C++后端侧代码，类。存放buffer_(存放数据以及其信息)、shape_（存放数据的shape信息）以及TensorDataType等信息。

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|ndata|ndarray|模型推理所需的输入数据|

**输出说明**

TensorBase类型对象，存储着模型推理的输入数据以及shape等信息。


#### to_device函数

**使用示例**

```python
device_id = 0
# 使用numpy数组构建aclruntime的Tensor类对象
tensor = aclruntime.Tensor(ndata)
# 将tensor数据放至npu上
tensor.to_device(device_id)
```

**功能说明**

TensorBase类函数。将数据从host侧移动到device侧，或者在不同device之间移动。

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|device_id|int|用于模型推理的Device的ID|

**输出说明**

无


#### to_host函数

**使用示例**

```python
# 将tensor数据由device侧放至host侧
tensor.to_host()
```

**功能说明**

TensorBase类函数。将数据从device侧移动到host侧。

**参数说明**

无

**输出说明**

无


#### run函数

**使用示例**

```python
session = aclruntime.InferenceSession(model_path, device_id, options)
# feeds是一个list，表示存储着模型推理输入所需的Tensor()构建的数据对象
# 构建模型推理所需的输出数据名称的list
outnames = [meta.name for meta in session.get_outputs()]
outputs = session.run(outnames, feeds)
```

**功能说明**

aclruntime.InferenceSession()实例对象运行模型推理的函数。

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|outnames|list|使用get_outputs()获得的输出数据信息中的name信息|
|feeds|list|存放aclruntime.Tensor类型数据，是模型推理所需数据|

**输出说明**

模型推理的结果，TensorBase类型数据。


#### sumary函数

**使用示例**

```python
session = aclruntime.InferenceSession(model_path, device_id, options)
outputs = session.run(outnames, feeds)
# 模型推理完成后，调用函数，可输出模型推理性能情况
session.sumary()
```

**功能说明**

获取模型推理的性能数据。

**参数说明**

无

**输出说明**

返回[[float, float]]类型数据。返回的list中按推理执行的先后顺序，保存了每一组数据推理的时间对（开始时间，结束时间）。


#### set_custom_outsize函数

**使用示例**

```python
# 动态shape模型需要使用，推理输出数据所占的内存大小(单位byte)
custom_sizes = 100000
custom_sizes = [custom_sizes] * len(outdesc)
session.set_custom_outsize(custom_sizes)
```

**功能说明**

动态shape场景所必须的函数，为模型推理输出数据预先分配custom_sizes中对应元素大小的内存。

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|custom_sizes|list[int]|每个模型输出预先分配的内存大小|

**输出说明**

无

### API场景说明

介绍利用aclruntime API推理使用频率最多的五种基本场景。

|编号<td rowspan='1'>**模型**<td rowspan='1'>**场景**</td><td rowspan='1'>**样例**</td><td rowspan='1'>**说明**</td>|
|----|
|1<td rowspan='5'>add_model</td><td rowspan='1'>static静态模型</td><td rowspan='1'>[aclruntime_api_static](#aclruntime-api-static)</td><td rowspan='1'>基本场景</td>|
|2<td rowspan='1'>dymbatch动态batch模型</td><td rowspan='1'>[aclruntime_api_dymbatch](#aclruntime-api-dymbatch)</td><td rowspan='1'>动态Batch，指定模型输入的实际Batch</td>|
|3<td rowspan='1'>dymhw动态分辨率模型</td><td rowspan='1'>[aclruntime_api_dymhw](#aclruntime-api-dymhw)</td><td rowspan='1'>动态分辨率，指定模型输入的实际H、W</td>|
|4<td rowspan='1'>dymdims动态维度模型</td><td rowspan='1'>[aclruntime_api_dymdims](#aclruntime-api-dymdims)</td><td rowspan='1'>动态维度，指定模型输入的实际shape</td>|
|5<td rowspan='1'>dymshape动态shape模型</td><td rowspan='1'>[aclruntime_api_dymshape](#aclruntime-api-dymshape)</td><td rowspan='1'>动态Shape，指定模型输入的实际shape</td>|

**add_model模型**：仅有一个加法算子的模型，获得两个tensor数据相加的结果。

如果要执行使用样例add_model，需要在linux环境下载[ais-bench_workload](https://gitee.com/ascend/tools/tree/master/ais-bench_workload)的源码，进入[使用样例目录](https://gitee.com/ascend/tools/tree/master/ais-bench_workload/tool/ais_bench/api_samples)下，执行以下命令生成样例执行所需的模型。

```python
chmod 750 get_sample_datas.sh
./get_sample_datas.sh
```

#### aclruntime API static

静态场景，以add_model模型为例，样例可执行文件在[aclruntime_api_static.py](api_samples/aclruntime_api_usage/aclruntime_api_static.py)。


#### aclruntime API dymbatch

动态batch场景，须自行设定batchsize。以add_model模型为例，样例可执行文件在[aclruntime_api_dymbatch.py](api_samples/aclruntime_api_usage/aclruntime_api_dymbatch.py)，示例代码中，调用`set_dynamic_batchsize()`执行设定batch的操作。

**设定batch**

```python
# set dynamic batch
indesc = session.get_inputs()
for i, shape in enumerate(shapes):
    for j, batchsize in enumerate(shape):
        if (indesc[i].shape[j] < 0):
            session.set_dynamic_batchsize(batchsize)
            print("input datas and intensors batchsize matched")
            break
        if (indesc[i].shape[j] != batchsize):
            raise RuntimeError("input datas and intensors batchsize not matched!")
```

##### set_dynamic_batchsize函数

**功能说明**

设定模型推理的batchsize

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|batchsize|int|表示模型推理的batchsize，根据该batchsize，对输入数据组batch|

**输出说明**

无


#### aclruntime API dymhw

动态分辨率场景，须自行设定自行设定分辨率（h、w）。以add_model模型为例，样例可执行文件在[aclruntime_api_dymhw.py](api_samples/aclruntime_api_usage/aclruntime_api_dymhw.py)，示例代码中，调用`set_dynamic_hw()`执行设定分辨率的操作。

**设定分辨率**

```python
# set dynamic HW
indesc = session.get_inputs()
for i, shape in enumerate(shapes):
    if (indesc[i].shape[2] < 0 and indesc[i].shape[3] < 0):
        session.set_dynamic_hw(shape[2], shape[3])
        break
```

##### set_dynamic_hw函数

**功能说明**

设定模型推理时输入的分辨率。

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|width|int|设定模型输入的imageSize的width|
|height|int|设定模型输入的imageSize的height|

**输出说明**

无

#### aclruntime API dymdims

动态维度场景，须自行设定维度。以add_model模型为例，样例可执行文件在[aclruntime_api_dymdims.py](api_samples/aclruntime_api_usage/aclruntime_api_dymdims.py)，示例代码中，调用`set_dynamic_dims()`执行设定维度的操作。

**设定维度**

```python
# set dynamic dims
dym_list = []
indesc = session.get_inputs()
for i, shape in enumerate(shapes):
    str_shape = [str(val) for val in shape]
    dyshape = "{}:{}".format(indesc[i].name, ",".join(str_shape))
    dym_list.append(dyshape)
dyshapes = ';'.join(dym_list)
session.set_dynamic_dims(dyshapes)
```

##### set_dynamic_dims函数

**功能说明**

设定模型推理时输入的维度。

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|dymdims|string|表示输入的维度信息，可设定输入的shape|

shape格式样例：

- 单输入

  inputs1:1,3,32,32

- 多输入

  inputs1:1,3,32,32;inputs2:4,3,32,32

**输出说明**

无

#### aclruntime API dymshape

动态shape场景，须自行设定模型输入的shape。以add_model模型为例，样例可执行文件在[aclruntime_api_dymshape.py](api_samples/aclruntime_api_usage/aclruntime_api_dymshape.py)，示例代码中，调用`set_dynamic_shape`执行设定shape的操作。

**设定shape**

```python
# set dynamic shape
dym_list = []
indesc = session.get_inputs()
for i, shape in enumerate(shapes):
    str_shape = [str(val) for val in shape]
    dyshape = "{}:{}".format(indesc[i].name, ",".join(str_shape))
    dym_list.append(dyshape)
dyshapes = ';'.join(dym_list)
session.set_dynamic_shape(dyshapes)
```

##### set_dynamic_shape函数

**功能说明**

设定模型推理时输入的shape

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|dymshape|string|表示输入的shape信息，可设定输入的shape|

shape格式样例（不同shape间使用`;`分割；一个shape中name和shape使用`:`分割；shape的具体大小之间使用`,`分割）：inputs1:1,3,32,32;inputs2:4,3,32,32

**输出说明**

无
