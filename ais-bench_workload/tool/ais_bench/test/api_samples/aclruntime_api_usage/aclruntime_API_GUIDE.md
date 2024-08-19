# ait benchmark interface python API使用指南
## benchmark API简介

AISBench通过在基于昇腾硬件的离线模型（.om模型）上运行推理功能，进行模型推理性能测试。用户可以通过[命令行前端](https://gitee.com/ascend/tools/tree/master/ais-bench_workload/tool/ais_bench#%E4%BD%BF%E7%94%A8%E6%96%B9%E6%B3%95)以及[Python API接口](https://gitee.com/ascend/tools/blob/master/ais-bench_workload/tool/ais_bench/API_GUIDE.md)执行模型的推理过程。这两种方法主要使用了Python端封装的类InferSession，调用其中封装的参数设置、模型推理函数，获得推理结果和性能测试情况。

aclruntime绑定了Python前端InferSession类和C++后端PyInferenceSession类，直接开放aclruntime推理API，使用户可以直接调用aclruntime模块，用Python函数代码直接调用模型推理后端的C++函数。

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
建立好模型推理的实例session后，准备outnames，表示模型输入结果的名称。在npu芯片上的数据和配置都已经设定完成，调用session的成员函数接口进行模型推理，接口返回值就是推理结果。
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
|1<td rowspan='5'>add_model</td><td rowspan='1'>static静态模型</td><td rowspan='1'>[aclruntime_api_static](#aclruntime-api-static)</td><td rowspan='1'>说明</td>|
|2<td rowspan='1'>dymbatch动态batch模型</td><td rowspan='1'>[aclruntime_api_dymbatch](#aclruntime-api-dymbatch)</td><td rowspan='1'>说明</td>|
|3<td rowspan='1'>dymhw动态宽高模型</td><td rowspan='1'>[aclruntime_api_dymhw](#aclruntime-api-dymhw)</td><td rowspan='1'>说明</td>|
|4<td rowspan='1'>dymdims动态维度模型</td><td rowspan='1'>[aclruntime_api_dymdims](#aclruntime-api-dymdims)</td><td rowspan='1'>说明</td>|
|5<td rowspan='1'>dymshape动态shape模型</td><td rowspan='1'>[aclruntime_api_dymshape](#aclruntime-api-dymshape)</td><td rowspan='1'>说明</td>|

add_model介绍：


### 通用函数说明

#### session_options函数

**功能说明**

**函数原型**

**返回值**

#### InferenceSession初始化实例函数

**功能说明**

**函数原型**

**参数说明**

**返回值**

#### get_inputs函数

**功能说明**

**函数原型**

**返回值**

#### Tensor函数

**功能说明**

**函数原型**

**参数说明**

**返回值**

#### to_device函数

**功能说明**

**函数原型**

**参数说明**

**返回值**

#### run函数

**功能说明**

**函数原型**

**参数说明**

**返回值**

#### to_host函数

**功能说明**

**函数原型**

**返回值**

#### sumary函数

**功能说明**

**函数原型**

**返回值**


### aclruntime API static
静态场景下，模型进行最基本的输入，运行推理，产生输出。以add_model模型为例。
#### 配置信息
```python
device_id = 0 # 模型推理的device ID
model_path = "../xxx_model.om" # om模型
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
    # convert acltenor to host memory
    out.to_host()
    # convert acltensor to numpy array
    outarray.append(np.array(out))
print(outarray)
# summay inference throughput
print("infer avg:{} ms".format(np.mean(session.sumary().exec_time_list)))
```

### aclruntime API dymbatch




### aclruntime API dymhw



### aclruntime API dymdims



### aclruntime API dymshape







<a name="InferSession1"></a>

### InferSession
#### 类原型
```python
class InferSession(device_id: int, model_path: str, acl_json_path: str = None, debug: bool = False, loop: int = 1)
```
#### 类说明
InferSession是**单进程**下用于om模型推理的类
#### 初始化参数
|参数名|说明|是否必选|
|----|----|----|
|**device_id**|uint8，npu芯片的id，在装了CANN驱动的服务器上使用`npu-smi info`查看可用的npu芯片的id。|是|
|**model_path**|str，om模型的路径，支持绝对路径和相对路径。|是|
|**acl_json_path**|str，acl json文件，用于配置profiling（采集推理过程详细的性能数据）和dump（采集模型每层算子的输入输出数据）|否|
|**debug**|bool，显示更详细的debug级别的log信息的开关，True为打开开关。|否|
|**loop**|int，一组输入数据重复推理的次数，至少为1。|否|

<a name="get_inputs1"></a>

#### <font color=#DD4466>**get_inputs函数**</font>
**功能说明**

用于获取InferSession加载的模型的输入节点的信息。

**函数原型**
```python
get_inputs()
```
**返回值**

返回类型为<font color=#44AA00>list [[aclruntime.tensor_desc](#acl_tensor_desc)]</font>的输入节点属性信息。

<a name="get_outputs1"></a>

#### <font color=#DD4466>**get_outputs函数**</font>
**功能说明**

用于获取InferSession加载的模型的输出节点的信息。

**函数原型**
```python
get_outputs()
```
**返回值**

返回类型为<font color=#44AA00>list [[aclruntime.tensor_desc](#acl_tensor_desc)]</font>的输出节点属性信息。 <br>
<a name="jump1"></a>

<a name="infer1"></a>

#### <font color=#DD4466>**infer函数**</font>

**功能说明**

模型推理接口，一次推理一组输入数据，可以推理静态shape、动态batch、动态分辨率、动态dims和动态shape场景的模型。

**函数原型**
```python
infer(feeds, mode='static', custom_sizes=100000, out_array=True)
```
**参数说明**
|参数名|说明|是否必选|
|----|----|----|
|**feeds**|推理所需的一组输入数据，支持数据类型:<a name="jump0"></a> <br> <ul>1、numpy.ndarray; <br> 2、单个numpy类型数据(np.int8, np.int16, np.int32, np.int64, np.uint8, np.uint16, np.uint32, np.float16, np.float32, np.float64); <br> 3、torch类型Tensor(torch.FloatTensor, torch.DoubleTensor, torch.HalfTensor, torch.BFloat16Tensor, torch.ByteTensor, torch.CharTensor, torch.ShortTensor, torch.LongTensor, torch.BoolTensor, torch.IntTensor) <br> 4、[aclruntime.Tensor](#acl_Tensor) </ul>|是|
|**mode**|str，指定加载的模型类型，可选'static'(静态模型)、'dymbatch'(动态batch模型)、'dymhw'(动态分辨率模型)、'dymdims'(动态dims模型)、'dymshape'(动态shape模型)|否|
|**custom_sizes**|int or [int]，动态shape模型需要使用，推理输出数据所占的内存大小(单位byte)。<br> <ul>1、输入为int时，模型的每一个输出都会被预先分配custom_sizes大小的内存。<br> 2、输入为list:[int]时, 模型的每一个输出会被预先分配custom_sizes中对应元素大小的内存。|否|
|**out_array**|bool，是否将模型推理的结果从device侧搬运到host侧|否|

**返回值**
+ out_array == True，返回numpy.ndarray类型的推理输出结果，数据的内存在host侧。
+ out_array == False，返回<font color=#44AA00>[aclruntime.Tensor](#acl_Tensor)</font>类型的推理输出结果，数据的内存在device侧。

<a name="jump3"></a> <a name="infer_pipeline1"></a>

#### <font color=#DD4466>**infer_pipeline函数**</font>

**功能说明**

多线程推理接口(计算与数据搬运在不同线程)，一次性推理多组数据建议采用此接口，相对于多次调用`infer`接口推理多组数据，可以有效缩短端到端时间。

**函数原型**
```python
infer_pipeline(feeds_list, mode = 'static', custom_sizes = 100000)
```

**参数说明**
|参数名|说明|是否必选|
|----|----|----|
|**feeds_list**|list，推理所需的几组组输入数据，list中支持数据类型:<a name="jump2"></a>: <br> <ul>1、numpy.ndarray; <br> 2、单个numpy类型数据(np.int8, np.int16, np.int32, np.int64, np.uint8, np.uint16, np.uint32, np.float16, np.float32, np.float64); <br> 3、torch类型Tensor(torch.FloatTensor, torch.DoubleTensor, torch.HalfTensor, torch.BFloat16Tensor, torch.ByteTensor, torch.CharTensor, torch.ShortTensor, torch.LongTensor, torch.BoolTensor, torch.IntTensor) <br> 4、[aclruntime.Tensor](#acl_Tensor) </ul><b>注意:</b><br> <ul>1、'static'、'dymbatch'和 'dymhw'场景下feeds_list中的每个feeds中shape必须相同 <br> 2、'dymdims'和 'dymshape'场景下feeds_list中的每个feeds中shape可以不相同|是|
|**mode**|str，指定加载的模型类型，可选'static'(静态模型)、'dymbatch'(动态batch模型)、'dymhw'(动态分辨率模型)、'dymdims'(动态dims模型)、'dymshape'(动态shape模型)|否|
|**custom_sizes**|int or [int]，动态shape模型需要使用，推理输出数据所占的内存大小(单位byte)。<ul><br>1、输入为int时，模型的每一个输出都会被预先分配custom_sizes大小的内存。<br>2、输入为list:[int]时，模型的每一个输出会被预先分配custom_sizes中对应元素大小的内存。|否·|

- **返回值**

返回list:[numpy.ndarray]类型的推理输出结果，数据的内存在host侧。

<a name="jump5"></a> <a name="infer_iteration1"></a>

#### <font color=#DD4466>**infer_iteration函数**</font>

**功能说明**

迭代推理接口，迭代推理(循环推理)指的是下一次推理的输入数据有部分来源于上一次推理的输出数据。相对于循环调用`infer`接口实现迭代推理，此接口可以缩短端到端时间。

**函数原型**
```python
infer_iteration(feeds, in_out_list = None, iteration_times = 1, mode = 'static', custom_sizes = 100000, mem_copy = True)
```

**参数说明**
|参数名|说明|是否必选|
|----|----|----|
|**feeds**|推理所需的一组输入数据，支持数据类型: <a name="jump4"></a> <br> <ul>1、numpy.ndarray; <br> 2、单个numpy类型数据(np.int8, np.int16, np.int32, np.int64, np.uint8, np.uint16, np.uint32, np.float16, np.float32, np.float64); <br> 3、torch类型Tensor(torch.FloatTensor, torch.DoubleTensor, torch.HalfTensor, torch.BFloat16Tensor, torch.ByteTensor, torch.CharTensor, torch.ShortTensor, torch.LongTensor, torch.BoolTensor, torch.IntTensor) <br> |是|
|**in_out_list**|[int]，表示每次迭代中，模型的输入来源于第几个输出，输入和输出的顺序与`get_inputs()`和`get_outputs()`获取的list中的元素顺序一致。例如，[-1, 1, 0]表示第一个输入数据复用原来的输入数据(用-1表示)，第二个输入数据来源于第二个输出数据，第三个输入来源于第一个输出数据。|是|
|**iteration_times**|int，迭代的次数。|否|
|**mode**|str，指定加载的模型类型，可选'static'(静态模型)、'dymbatch'(动态batch模型)、'dymhw'(动态分辨率模型)、'dymdims'(动态dims模型)、'dymshape'(动态shape模型)|否|
|**custom_sizes**|int or [int]，动态shape模型需要使用，推理输出数据所占的内存大小(单位byte)。<br><ul> 1、输入为int时，模型的每一个输出都会被预先分配custom_sizes大小的内存。<br> 2、输入为list:[int]时，模型的每一个输出会被预先分配custom_sizes中对应元素大小的内存。|否|

- **返回值**

返回numpy.ndarray类型的推理输出结果，数据的内存在host侧。

<a name="summary1"></a>

#### <font color=#DD4466>**summary函数**</font>

**功能说明**

用于获取推理过程的性能数据。

**函数原型**
```python
summary()
```
**返回值**

返回[[float, float]]类型的数据。返回的list中按推理执行的先后顺序，保存了每一组数据推理的时间对（开始时间，结束时间）。

<a name="reset_summaryinfo1"></a>

#### <font color=#DD4466>**reset_summaryinfo函数**</font>

**功能说明**

用于清空`summary()`获取的性能数据。

**函数原型**
```python
reset_summaryinfo()
```
**返回值**

无

<a name="free_resource1"></a>

#### <font color=#DD4466>**free_resource函数**</font>

**功能说明**

用于释放InferSession相关的device侧资源，但是不会释放InferSession对应device内InferSession所在进程内和AscendCL相关的其他资源。

**函数原型**
```python
free_resource()
```

**返回值**

无

<a name="finalize1"></a>

#### <font color=#DD4466>**finalize函数**</font>
**功能说明**

用于释放InferSession对应device内InferSession所在进程和AscendCL相关的所有资源。**注意是类方法**，通过`InferSession.finalize()`方式调用，推荐在进程结束前手动调用，调用后在当前进程无法再执行device上的任务。

**函数原型**
```python
finalize()
```

**返回值**

无

<a name="MultiDeviceSession1"></a>

### MultiDeviceSession
#### 类原型
```python
class MultiDeviceSession(model_path: str, acl_json_path: str = None, debug: bool = False, loop: int = 1)
```
#### 类说明
MultiDeviceSession是**多进程**下用于om模型推理的类，初始化时不会在npu芯片(device)上加载模型，使用推理接口时才会在指定的几个devices的每个进程中新建一个InferSession。
#### 初始化参数
|参数名|说明|是否必选|
|----|----|----|
|**model_path**|str，om模型的路径，支持绝对路径和相对路径。|是|
|**acl_json_path**|str，acl json文件，用于配置profiling（采集推理过程详细的性能数据）和dump（采集模型每层算子的输入输出数据）。|否|
|**debug**|bool，显示更详细的debug级别的log信息的开关，True为打开开关。|否|
|**loop**|int，一组输入数据重复推理的次数，至少为1。|否|

<a name="infer2"></a>

#### <font color=#DD4466>**infer函数**</font>
**功能说明**

多进程调用InferSession的[infer接口](#jump1)进行推理

**函数原型**
```python
infer(devices_feeds, mode='static', custom_sizes=100000)
```

**参数说明**
|参数名|说明|是否必选|
|----|----|----|
|**devices_feeds**|dict，{device_id: [feeds1, feeds2, ...]}，device_id对应的device中的每个feeds都会单独开一个进程推理，feeds的定义参考[InferSession的infer接口中对feeds的定义](#jump0)|是|
|**mode**|str，指定加载的模型类型，可选'static'(静态模型)、'dymbatch'(动态batch模型)、'dymhw'(动态分辨率模型)、'dymdims'(动态dims模型)、'dymshape'(动态shape模型)|否|
|**custom_sizes**|int or [int]，动态shape模型需要使用，推理输出数据所占的内存大小(单位byte)<br><ul> 1、输入为int时，模型的每一个输出都会被预先分配custom_sizes大小的内存。<br> 2、输入为list:[int]时, 模型的每一个输出会被预先分配custom_sizes中对应元素大小的内存。|否|

**返回值**
返回{device_id:[output1, output2, ...]}，output*为numpy.ndarray类型的推理输出结果，数据的内存在host侧。

<a name="infer_pipeline2"></a>

#### <font color=#DD4466>**infer_pipeline函数**</font>

**功能说明**

多进程调用InferSession的[infer_pipeline接口](#jump3)进行推理。

**函数原型**
```python
infer_pipeline(devices_feeds_list, mode = 'static', custom_sizes = 100000)
```

**参数说明**
|参数名|说明|是否必选|
|----|----|----|
|**devices_feeds_list**|dict，{device_id: [feeds_list1, feeds_list2, ...]}，device_id对应的device的每个feeds_list都会单独开一个进程推理，feeds_list的定义参考[InferSession的infer_pipeline接口中对feeds_list的定义](#jump2)。|是|
|**mode**|str，指定加载的模型类型，可选'static'(静态模型)、'dymbatch'(动态batch模型)、'dymhw'(动态分辨率模型)、'dymdims'(动态dims模型)、'dymshape'(动态shape模型)|否|
|**custom_sizes**|int or [int]，动态shape模型需要使用，推理输出数据所占的内存大小(单位byte)。<ul><br> 1、输入为int时，模型的每一个输出都会被预先分配custom_sizes大小的内存。<br> 2、输入为list:[int]时，模型的每一个输出会被预先分配custom_sizes中对应元素大小的内存。|否|

**返回值**
返回{device_id:[output1, output2, ...]}，output*为[numpy.ndarray]类型的推理输出结果，数据的内存在host侧。

<a name="infer_iteration2"></a>

#### <font color=#DD4466>**infer_iteration函数**</font>

**功能说明**

多进程调用InferSession的[infer_iteration接口](#jump5)进行推理。

**函数原型**
```python
infer_iteration(device_feeds, in_out_list = None, iteration_times = 1, mode = 'static', custom_sizes = None, mem_copy = True)
```

**参数说明**
|参数名|说明|是否可选|
|----|----|----|
|**devices_feeds**|dict，{device_id: [feeds1, feeds2, ...]}，device_id对应的device的每个feeds都会单独开一个进程推理，feeds的定义参考[InferSession的infer_iteration接口中对feeds的定义](#jump4)。|是|
|**in_out_list**|[int]，表示每次迭代中，模型的输入来源于第几个输出，输入和输出的顺序与`get_inputs()`和`get_outputs()`获取的list中的元素顺序一致。例如，[-1, 1, 0]表示第一个输入数据复用原来的输入数据(用-1表示)，第二个输入数据来源于第二个输出数据，第三个输入来源于第一个输出数据。|是|
|**iteration_times**|int，迭代的次数。|否|
|**mode**|str，指定加载的模型类型，可选'static'(静态模型)、'dymbatch'(动态batch模型)、'dymhw'(动态分辨率模型)、'dymdims'(动态dims模型)、'dymshape'(动态shape模型)|否|
|**custom_sizes**|int or [int]，动态shape模型需要使用，推理输出数据所占的内存大小(单位byte)。<ul><br> 1、输入为int时，模型的每一个输出都会被预先分配custom_sizes大小的内存。<br> 2、输入为list:[int]时，模型的每一个输出会被预先分配custom_sizes中对应元素大小的内存。|否|

**返回值**

返回{device_id:[output1, output2, ...]}，output*为numpy.ndarray类型的推理输出结果，数据的内存在host侧。

<a name="summary2"></a>

#### <font color=#DD4466>**summary函数**</font>

**功能说明**

获取最近一次使用多进程推理接口得到的端到端推理时间(包含模型加载时间)。

**函数原型**

```python
summary()
```

**返回值**

返回{device_id:[e2etime1, e2etime2, ...]}，e2etime*为每个进程端到端推理的时间(包含模型加载时间)。

<a name="MemorySummary1"></a>

### MemorySummary
#### 类原型
```python
MemorySummary()
```
#### 类说明
MemorySummary是用于统计一个推理进程中host2device和device2host过程的拷贝时间。

<a name="get_h2d_time_list1"></a>

#### <font color=#DD4466>**get_h2d_time_list函数**</font>

**功能说明**

获取整个进程中所有的host2device过程的拷贝时间。

**函数原型**
```python
get_h2d_time_list()
```
**返回值**

返回[float]类型的数据。返回的list中的时间，按推理执行的先后顺序排序。

<a name="get_d2h_time_list1"></a>

#### <font color=#DD4466>**get_d2h_time_list函数**</font>
**功能说明**

获取整个进程中所有的device2host过程的拷贝时间。

**函数原型**
```python
get_d2h_time_list()
```

**返回值**

返回[float]类型的数据。返回的list中的时间，按推理执行的先后顺序排序。

<a name="reset1"></a>

#### <font color=#DD4466>**reset函数**</font>
**功能说明**

用于清空`get_h2d_time_list`和`get_d2h_time_list`获取的数据。

**函数原型**
```python
reset()
```

**返回值**

无

### 内部数据类型解释

<a name="acl_tensor_desc"></a>

#### <font color=#DD4466>**aclruntime.tensor_desc**</font>
描述模型输入输出节点信息的结构体：<br>
- property <font color=#DD4466>**name**</font>:str
    + 节点名称。
- property <font color=#DD4466>**datatype**</font>:[aclruntime.dtype](#acl_dtype)
    + 节点接受tensor的数据类型
- property <font color=#DD4466>**format**</font>:int
    + 节点接受tensor格式，0表示NCHW格式，1表示NHWC格式。
- property <font color=#DD4466>**shape**</font>:list [int]
    + 节点接受的tensor的shape。
- property <font color=#DD4466>**size**</font>:int
    + 节点接受的tensor的大小。
- property <font color=#DD4466>**realsize**</font>:int
    + 节点接受的tensor的真实大小，针对动态shape 动态分档场景 实际需要的大小。

<a name="acl_dtype"></a>

#### <font color=#DD4466>**aclruntime.dtype**</font>(enum)
数据类型名称的枚举类型：<br>
- 包含 'uint8', 'int8', 'uint16', 'int16', 'uint32', 'int32', 'uint64', 'int64', 'float16', 'float32', 'double64', 'bool'

<a name="acl_Tensor"></a>

#### <font color=#DD4466>**aclruntime.Tensor**</font>
- device侧保存tensor的方式，在host侧无法直接访问

## interface python API 使用样例
- 如果要执行使用样例，需要在linux环境下载[ait](https://gitee.com/ascend/tools)的源码，进入[使用样例目录](https://gitee.com/ascend/tools/ais-bench_workload/tool/ais_bench/api_samples)下, 执行以下命令生成样例执行所需的模型（仅支持在310系列的推理卡上生成，不支持在910系列的训练卡上生成）。
  ```cmd
  chmod 750 get_sample_datas.sh
  ```
  ```cmd
  ./get_sample_datas.sh
  ```

### 样例列表
#### 单进程使用`InferSession.infer`接口推理
|样例|说明|
| ---- | ---- |
|[infer_api_static.py](api_samples/interface_api_usage/api_infer/infer_api_static.py)|调用InferSession的infer接口推理静态模型|
|[infer_api_dymbatch.py](api_samples/interface_api_usage/api_infer/infer_api_dymbatch.py)|调用InferSession的infer接口推理动态batch模型|
|[infer_api_dymhw.py](api_samples/interface_api_usage/api_infer/infer_api_dymhw.py)|调用InferSession的infer接口推理动态分辨率模型|
|[infer_api_dymdims.py](api_samples/interface_api_usage/api_infer/infer_api_dymdims.py)|调用InferSession的infer接口推理动态dims模型|
|[infer_api_dymshape.py](api_samples/interface_api_usage/api_infer/infer_api_dymshape.py)|调用InferSession的infer接口推理动态shape模型|

#### 单进程使用`InferSession.infer_pipeline`接口推理
|样例|说明|
| ---- | ---- |
|[infer_pipeline_api_static.py](api_samples/interface_api_usage/api_infer_pipeline/infer_pipeline_api_static.py)|调用InferSession的infer_pipeline接口推理静态模型|
|[infer_pipeline_api_dymbatch.py](/api_samples/interface_api_usage/api_infer_pipeline/infer_pipeline_api_dymbatch.py)|调用InferSession的infer_pipeline接口推理动态batch模型|
|[infer_pipeline_api_dymhw.py](api_samples/interface_api_usage/api_infer_pipeline/infer_pipeline_api_dymhw.py)|调用InferSession的infer_pipeline接口推理动态分辨率模型|
|[infer_pipeline_api_dymdims.py](api_samples/interface_api_usage/api_infer_pipeline/infer_pipeline_api_dymdims.py)|调用InferSession的infer_pipeline接口推理动态dims模型|
|[infer_pipeline_api_dymshape.py](api_samples/interface_api_usage/api_infer_pipeline/infer_pipeline_api_dymshape.py)|调用InferSession的infer_pipeline接口推理动态shape模型|

#### 单进程使用`InferSession.infer_iteration`接口推理
|样例|说明|
| ---- | ---- |
|[infer_iteration_api_static.py](api_samples/interface_api_usage/api_infer_iteration/infer_iteration_api_static.py)|调用InferSession的infer_iteration接口推理静态模型|
|[infer_iteration_api_dymbatch.py](api_samples/interface_api_usage/api_infer_iteration/infer_iteration_api_dymbatch.py)|调用InferSession的infer_iteration接口推理动态batch模型|
|[infer_iteration_api_dymhw.py](api_samples/interface_api_usage/api_infer_iteration/infer_iteration_api_dymhw.py)|调用InferSession的infer_iteration接口推理动态分辨率模型|
|[infer_iteration_api_dymdims.py](api_samples/interface_api_usage/api_infer_iteration/infer_iteration_api_dymdims.py)|调用InferSession的infer_iteration接口推理动态dims模型|
|[infer_iteration_api_dymshape.py](api_samples/interface_api_usage/api_infer_iteration/infer_iteration_api_dymshape.py)|调用InferSession的infer_iteration接口推理动态shape模型|

#### 多进程使用推理接口推理
|样例|说明|
| ---- | ---- |
|[multidevice_infer_api.py](api_samples/interface_api_usage/multidevice_api/multidevice_infer_api.py)|调用MultiDeviceSession的infer接口推理静态模型|
|[multidevice_infer_pipeline_api.py](api_samples/interface_api_usage/multidevice_api/multidevice_infer_pipeline_api.py)|调用MultiDeviceSession的infer_pipeline接口推理静态模型|
|[multidevice_infer_iteration_api.py](benchmark/api_samples/interface_api_usage/multidevice_api/multidevice_infer_iteration_api.py)|调用MultiDeviceSession的infer_iteration接口推理静态模型|

