# 单机多卡使用注意事项及常见问题

## 注意事项
单机多卡使用精度工具相对单机单卡使用多了一些注意事项，如果你已经熟悉单机单卡使用精度工具，那么阅读本注意事项可以使你快速上手正确使用工具的单机多卡功能。

### set_dump_path 注意事项
单机多卡一般是多进程实现的，须保证每个进程都正确地set_dump_path。如果你不知道如何设置，可以把set_dump_path 直接放在import语句后，如：
```python
from ptdbg_ascend import *
seed_all()
set_dump_path(’./dump_resnet/myDump.pkl')
```
这样可以保证set_dump_path在每个进程上都被调用。

[//]: # (需要指出的是，由于dump数据非多进程安全，我们改变了[dump文件夹格式]&#40;#dump文件夹格式&#41;。)

### register_hook 注意事项
1. register_hook 需要在set_dump_path之后调用，也需要在每个进程上被调用，并且最好在模型数据被搬运到卡上之后再调用。有几种识别方法：

    a) 如果你能找到训练代码中遍历epoch的for循环或者遍历数据集的for循环，那么把register_hook 放到那个循环开始前即可。

    b) 如果你能找到训练代码中调用DDP 或者DistributedDataParallel 的代码行，那么把register_hook 放到这行所在的代码块之后即可
如果按照以上两种方式之一配置register_hook，那么工具就可以正常运行了。如果没有找到，那么尽可能把这行代码往后放，并且考虑配置下面的rank 参数。

2. rank参数

register_hook新增了一个`rank`参数，工具会根据`rank`来创建dump数据的文件夹。如果不传入`rank`，工具会从模型参数中读取device_id 卡号作为`rank`。
如果你已经按照上一条规则配置了register_hook，工具就已经可以从模型参数中读取到卡号了，那么你大概可以忽略本条注意事项。

#### 如何配置rank参数
如果你知道运行时当前进程的对应卡的`rank_id`编号，可以将其作为`rank`参数传入。比如
```python
 # rank_id 为运行时当前进程的rank编号, 第二个参数以实际需要的功能为准，可以用overflow_check替换
register_hook(model, acc_cmp_dump, rank=rank_id)
```

**方式一：环境变量**

当前进程的rank_id有时会保存在环境变量中，比如`LOCAL_RANK`。你可以通过
```python
import os
print("Local rank is: ", os.environ.get('LOCAL_RANK'))
```
来检查当前进程的rank_id。如果打印结果显示该环境变量有被配置过，如
```commandline
# 以单机8卡为例，顺序不重要，有数字就行
Local rank is: 0
Local rank is: 2
Local rank is: 3
Local rank is: 1
Local rank is: 4
Local rank is: 5
Local rank is: 6
Local rank is: 7
```
那么你可以把这个环境变量作为rank传参，如
```python
# 须先import os
register_hook(model, acc_cmp_dump, rank=os.environ.get('LOCAL_RANK')
```
** 方式二：命令行参数 **

有时会通过命令行参数传入rank_id，比如`--local_rank`。那样你应该可以在代码中找到`args.local_rank` 来作为rank参数值。比如
```python
register_hook(model, acc_cmp_dump, rank=args.local_rank)
```

### Dump文件夹格式
我们模仿ACL溢出检测dump的文件夹，区分了不同rank所dump的数据文件。假设dump路径设置为`set_dump_path('./dump_path/myDump.pkl', dump_tag='dump_conv2d')`
（`dump_tag`是新增的参数）， 则数据（pkl和包含npy文件的文件夹）会dump在：`./dump_path/{dump_tag}_{version}/rank{rank_id}/`路径下。比如：

  ```
  ├── dump_path
  │   └── dump_conv2d_v1.0
  │       ├── rank0
  │       │   ├── myDump
  |       |   |    ├── Tensor_permute_1_forward.npy
  |       |   |    ...
  |       |   |    └── Fcuntion_linear_5_backward_output.npy
  │       │   └── myDump.pkl
  │       ├── rank1
  |       |   ├── myDump
  |       |   |   └── ...
  |       |   └── myDump.pkl 
  │       ├── rank2
  |       |   ├── myDump
  |       |   |   └── ...
  |       |   └── myDump.pkl 
  │       ├── ...
  │       |
  |       └── rank7
  ```

具体地说，dump_path下首先产生一个`{dump_tag}_{version}`文件夹，`dump_tag`是set_dump_path传入参数设置的，可以用来提高文件夹辨识度，
默认值为`ptdbg_dump`（因此默认情况下你可以看到一个名为`ptdbg_dump_v1.0`的文件夹）。
`version`是工具版本，用于区分不同版本工具所dump的数据这个文件夹中会根据实际使用卡的数量产生若干`rank`文件夹。
每张卡上dump结果产生pkl和npy数据文件夹会存在对应的rank文件夹下。
需要注意的是，如果以相同的dump_path和dump_tag运行两次，则**第二次的数据文件会覆盖第一次的**。

## 常见问题
1. dump文件夹不符合上面的格式，明明是多卡运行，却只有一个rank0文件夹；或者，dump出来的pkl文件格式损坏，有些行的内容不完整。
这通常是因为register_hook没有正确配置，工具没有获取正确的`rank_id`（从rank 参数读取或从模型参数的device_id读取），请参考
[register_hook注意事项](#registerhook-注意事项)来正确配置。


2. HCCL 报错， error code: EI0006
这是在部分较旧版本CANN包上可能会发生的，如果你在使用工具时发现这个错误（同时不使用工具没有错误），可以考虑升级CANN版本。

3. 溢出检测`overflow_nums`功能没有正常运行
目前由于多进程的限制，溢出次数是在各进程上单独计数的，而不是所有进程共享一个计数器。训练会在某一张卡（某一个进程）上的溢出次数达到`overflow_nums`时停止。
