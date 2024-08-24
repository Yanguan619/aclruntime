# aclruntime API使用指南
## API简介

AISBench通过在基于昇腾硬件的离线模型（.om模型）上运行推理功能，进行模型推理性能测试。用户可以通过[命令行前端](https://gitee.com/ascend/tools/tree/master/ais-bench_workload/tool/ais_bench#%E4%BD%BF%E7%94%A8%E6%96%B9%E6%B3%95)以及[Python API接口](https://gitee.com/ascend/tools/blob/master/ais-bench_workload/tool/ais_bench/API_GUIDE.md)执行模型的推理过程。这两种方法主要使用了Python端封装的类InferSession，调用其中封装的参数设置、模型推理函数，获得推理结果和性能测试情况。

aclruntime绑定了Python前端InferSession类和C++后端PyInferenceSession类，直接开放aclruntime推理API，使用户可以直接调用aclruntime模块，用Python函数代码直接调用模型推理后端的C++函数，减少了在python端的一些操作，提升模型推理和开发的效率。

使用ais_bench推理工具提供的api需要安装`ais_bench`和`aclruntime`包。安装方法参考[ais_bench推理工具使用指南](https://gitee.com/ascend/tools/blob/master/ais-bench_workload/tool/ais_bench/README.md)的“工具安装”章节。

## aclruntime API 基本流程

### 导入依赖包
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
# 将数据放至npu上
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
exec_time = session.summary().exec_time_list[-1]
```

## aclruntime API 使用场景
介绍利用aclruntime API推理使用频率最多的五种基本场景
### API场景说明
|编号<td rowspan='1'>**模型**<td rowspan='1'>**场景**</td><td rowspan='1'>**样例**</td><td rowspan='1'>**说明**</td>|
|----|
|1<td rowspan='5'>add_model</td><td rowspan='1'>static静态模型</td><td rowspan='1'>[aclruntime_api_static](#aclruntime-api-static)</td><td rowspan='1'>基本场景</td>|
|2<td rowspan='1'>dymbatch动态batch模型</td><td rowspan='1'>[aclruntime_api_dymbatch](#aclruntime-api-dymbatch)</td><td rowspan='1'>动态Batch，指定模型输入的实际Batch</td>|
|3<td rowspan='1'>dymhw动态分辨率模型</td><td rowspan='1'>[aclruntime_api_dymhw](#aclruntime-api-dymhw)</td><td rowspan='1'>动态分辨率，指定模型输入的实际H、W</td>|
|4<td rowspan='1'>dymdims动态维度模型</td><td rowspan='1'>[aclruntime_api_dymdims](#aclruntime-api-dymdims)</td><td rowspan='1'>动态维度，指定模型输入的实际shape</td>|
|5<td rowspan='1'>dymshape动态shape模型</td><td rowspan='1'>[aclruntime_api_dymshape](#aclruntime-api-dymshape)</td><td rowspan='1'>动态Shape，指定模型输入的实际shape</td>|

**add_model模型**：仅有一个加法算子的模型，获得两个tensor数据相加的结果。


### 通用函数说明

#### session_options函数

**功能说明**

创建一个包含日志等级（`log_level`）、循环推理次数（`loop`）和配置文件地址（`aclJsonPath`）的对象。

**参数说明**

无

**输出说明**

输出模型推理所需的数据信息的实例。


#### InferenceSession初始化实例函数

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

**功能说明**

使用推理实例调用。用于获取aclruntime.InferenceSession()加载的模型的输入节点信息。

**参数说明**

无

**输出说明**

输出list[aclruntime.TensorDesc]类型数据，包括输入数据属性信息。
TensorDesc:C++后端侧代码，结构体。存放name、TensorDataType、输入数据shapes、输入数据size等类型信息。


#### get_outputs函数

**功能说明**

使用推理实例调用。用于获取aclruntime.InferenceSession()加载的模型的输出节点信息。

**参数说明**

无

**输出说明**

输出list[aclruntime.TensorDesc]类型数据，包括输出数据属性信息。


#### Tensor函数

**功能说明**

使用aclruntime.Tensor()调用。根据numpy类型的数据构造TensorBase类对象，存储模型推理所需的数据和信息。
TensorBase：C++后端侧代码，类。存放buffer_(存放数据以及其信息)、shape_（存放数据的shape信息）以及TensorDataType等信息。

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|ndata|ndarray|模型推理所需的输入数据|

**输出说明**

TensorBase类型对象，存储着模型推理的输入数据以及shape等信息。


#### to_device函数

**功能说明**

TensorBase类函数。将数据从host侧移动到device侧，或者在不同device之间移动。

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|device_id|int|模型推理npu的ID|

**输出说明**

无


#### to_host函数

**功能说明**

TensorBase类函数。将数据从device侧移动到host侧。

**参数说明**

无

**输出说明**

无


#### run函数

**功能说明**

aclruntime.InferenceSession()实例对象运行模型推理的函数。

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|outnames|list|使用get_outputs()获得的输出数据信息中的name信息|
|feeds|list|存放aclruntime.Tensor类型数据，是模型推理所需数据|

**输出说明**

模型推理的结果，TenSorBase类型数据。


#### sumary函数

**功能说明**

获取模型推理的性能数据。

**参数说明**

无

**输出说明**

返回[[float, float]]类型数据。返回的list中按推理执行的先后顺序，保存了每一组数据推理的时间对（开始时间，结束时间）。


### aclruntime API static

静态场景下，模型进行固定形式的输入，运行推理，产生输出。以**add_model**模型为例。在该场景下，仅需要构建模型所需shape的数据，将其迁移至npu上，然后输入到模型推理接口，即可运行模型的推理，获取模型推理结果，也可以查看模型推理的性能信息。

#### 配置信息
```python
device_id = 0 # 模型推理的device ID
model_path = "../xxx/add_model.om" # om模型
```

#### 数据准备
```python
shape0 = session.get_inputs()[0].shape
ndata0 = np.full(shape0, 1).astype(np.float32)
shape1 = session.get_inputs()[1].shape
ndata1 = np.full(shape1, 1).astype(np.float32)
```

#### 数据迁移
```python
# 将数据迁移到对应编号的npu上
feeds = []
tensor0 = aclruntime.Tensor(ndata0)
tensor0.to_device(device_id)
feeds.append(tensor0)
tensor1 = aclruntime.Tensor(ndata1)
tensor1.to_device(device_id)
feeds.append(tensor1)
```

#### 模型推理
```python
outnames = [meta.name for meta in session.get_outputs()]
outputs = session.run(outnames, feeds)
```

#### 结果查看
```python
print(f"outputs: {outputs}")
outarray = []
for out in outputs:
    # 将acltenor移动到host侧
    out.to_host()
    # 将acltenor转换为ndarray数据
    outarray.append(np.array(out))
print(outarray)
# 模型推理吞吐量
print("infer avg:{} ms".format(np.mean(session.sumary().exec_time_list)))
```


### aclruntime API dymbatch

动态batch场景，设定模型的batchsize，将输入数据按照设定的batchsize组batch，运行模型推理，产生输出。以add_model模型为例。

模型的**配置信息**、**数据准备**、**数据迁移**、**模型推理**以及**结果**查看都与[aclruntime API static](#aclruntime-api-static)场景一致。

若模型推理时包含动态Batch特性，在模型推理时，要设置模型推理时需使用的batch size，模型支持的batch size已提前在构建模型时配置（使用ATC工具的dynamic_batch_size参数）。在动态batch的场景下，自行设定batchsize，根据该batchsize组成batch，运行模型推理。示例代码中，调用`set_dynamic_batchsize()`执行设定batch的操作。

#### 设定batch
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

#### set_dynamic_batchsize函数

**功能说明**

设定模型推理的batchsize

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|batchsize|int|表示模型推理的batchsize，根据该batchsize对输入数据组batch|

**输出说明**

无


### aclruntime API dymhw

动态分辨率场景，设定模型输入数据的分辨率，运行模型推理，产生输出。以add_model模型为例。

模型的**配置信息**、**数据准备**、**数据迁移**、**模型推理**以及**结果**查看都与[aclruntime API static](#aclruntime-api-static)场景一致。

若模型推理时包含动态分辨率特性，在模型推理时，要设置模型推理时需使用的分辨率，模型支持的分辨率已提前在构建模型时配置（使用ATC工具的dynamic_image_size参数）。在动态分辨率的场景下，自行设定h、w，运行模型推理。示例代码中，调用`set_dynamic_hw()`执行设定分辨率的操作。

#### 设定分辨率
```python
# set dynamic HW
indesc = session.get_inputs()
for i, shape in enumerate(shapes):
    if (indesc[i].shape[2] < 0 and indesc[i].shape[3] < 0):
        session.set_dynamic_hw(shape[2], shape[3])
        break
```

#### set_dynamic_hw函数

**功能说明**

设定模型推理时输入数据的分辨率

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|width|int|设定模型输入数据的imageSize的width|
|height|int|设定模型输入数据的imageSize的height|

**输出说明**

无


### aclruntime API dymdims

动态维度场景，设定模型输入数据的维度，运行模型推理，产生输出。以add_model模型为例。

模型的**配置信息**、**数据准备**、**数据迁移**、**模型推理**以及**结果**查看都与[aclruntime API static](#aclruntime-api-static)场景一致。

若模型推理时包含动态维度特性，在模型推理时，要设置模型推理时需使用的维度值，模型支持哪些维度值已提前在构建模型时配置（使用ATC工具的dynamic_dims参数）。在动态维度的场景下，自行设定维度，运行模型推理。示例代码中，调用`set_dynamic_dims()`执行设定维度的操作。

#### 设定维度
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

#### set_dynamic_dims函数

**功能说明**

设定模型推理时输入数据的维度

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|dymdims|string|表示输入数据的维度信息，可设定输入数据的shape|

**输出说明**

无


### aclruntime API dymshape

动态shape场景，设定模型输入数据的shape，运行模型推理，产生输出。以add_model模型为例。

模型的**配置信息**、**数据准备**、**数据迁移**、**模型推理**以及**结果**查看都与[aclruntime API static](#aclruntime-api-static)场景一致。

若模型推理时包含动态shape的特性，在模型推理时，需要设置模型推理时固定的shape，模型支持的shape情况，已提前在构建模型时配置（使用ATC工具，通过input_shape参数设置输入Shape范围）。在动态shape场景下，设定模型输入数据的shape，根据该shape输入数据，并运行模型推理。示例代码中，调用`set_dynamic_shape`执行设定shape的操作。

#### 设定维度
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

#### set_dynamic_shape函数

**功能说明**

设定模型推理时输入数据的shape

**参数说明**

|**参数**|**类型**|**说明**|
|--------|--------|--------|
|dymshape|string|表示输入数据的shape信息，可设定输入数据的shape|

**输出说明**

无