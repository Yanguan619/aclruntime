# ptdbg_ascend
# **PyTorch精度工具**

## **使用场景**

在同一模型或算子调试过程中，遇到算子相关的计算精度问题，定位时费时费力，所以推出了一个精度比对工具。

精度对比工具，通过在PyTorch模型中注入hook，跟踪计算图中算子的前向传播与反向传播时的输入与输出，排查存在计算精度误差，进行问题的精准定位。

主要的使用场景包括：

- 同一模型，从cpu(或gpu)移植到npu中存在精度下降问题，对比npu芯片中的算子计算数值与cpu/gpu芯片中的算子计算数值，进行问题定位。
- 同一模型，进行迭代(模型、算子或设备迭代)时存在的精度下降问题，对比相同模型在迭代前后版本的算子计算数值，进行问题定位。

## **例外场景**
1. 当前dump默认只会dump tensor类型数据，且会判断tensor是否为浮点类型，只Dump浮点型，若要dump 布尔，整型tensor以及标量请参考Doc目录下文档，关闭数据过滤开关filter_switch="OFF"，请用户根据需要设置，此选项默认不需要用户设置且默认值为ON。
2. 只支持dump反向调用链路中的算子，如果不在链路中会出现只有前向没有反向dump数据的情况。

## **精度比对基本原理**

普遍适用的方法是以模型为单位，采用hook机制挂在模型的上。当模型在CPU上进行正向传播时跟踪并dump每一层的数值输入与输出，在反向传播时跟踪并dump每一层的梯度输入值与输出值；同样的当模型在NPU中进行计算时采用相同的方式记录下相应的数据，通过对比dump出的数值，计算余弦相似度和均方根误差的方式,
定位和排查NPU算子存在的计算精度问题。

精度比对工具dump数据说明：在实际使用场景中网络模型通常较大，基于整网全量数据的dump，耗时长且储存文件大。因此内部默认使用部分网络参数和统计量的方式dump数据来提升效率。如需dump全量数据，请将register_hook函数中的sample参数设为False（默认为True）。

![图1：精度比对逻辑图](figures/module_compare.png)

图1即为精度对比的基本逻辑，思路清晰明了，但其中存在较多的细节问题：

1. 需要对大量的变量进行控制，要确保模型结构参数等相同。
2. 相同的模型在不同的硬件设备上进行运算时可能会出现相同的计算会调用不同的底层算子，造成npu算子可能出现的不匹配情形。
3. NPU与CPU/GPU的计算结果误差可能会随着模型的执行不断累积，最终会出现同一个算子因为输入的数据差异较大而无法匹配对比计算精度的情况。
4. 该比对机制是以GPU/CPU侧api输入输出为标杆数据，进行比对。因此对于NPU侧自研api，无法找到对应的标杆数据，因此该机制暂不支持此场景的数据dump与比对。

其中细节问题2可能表现如下图2：

![图2：算子映射匹配](figures/op_compare.png)

由于可能会出现融合算子，所以在算子的逐一匹配时可能会出现错误匹配或无法匹配的问题，例如图2中NPU算子npu_op_1与npu_op_2无法和cpu_op_k进行匹配，才去跳过的方式，直到到npu_op_3和cpu_op_3才从新对齐开始匹配。

### **算子匹配条件**

判断运行在cpu和npu上的两个算子是否相同采用的步骤如下：
1. 两个算子的全名是否相同，算子命名规则如下：`{api_type}_{api_name}_{api调用次数}_{正反向}_{输入输出}.index`, 如：Functional_conv2d_1_backward_input.0
2. 两个算子的输入输出Tensor数量和各个Tensor的shape是否相同

通常满足以上的两个条件，就认为是同一个算子，成功进行算子的匹配，后续进行相应的计算精度比对。



## 环境准备

安装了Pytorch 1.8.1 或者 Pytorch 1.11.0版本的Linux系统。

- Linux OS

## 安装

### 发布包安装
您可以从以下链接获取到精度工具的发布包

| ptdbg_ascend版本 | 发布日期      | 支持PyTorch版本 | 下载链接                                                                                                                                | 参考文档                                       |
|----------------|-----------|------|-------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------|
| 1.0            | 2023-3-30 |1.8.1/1.11.0| [ptdbg_ascend-1.0-py3-none-any.whl](https://ptdbg.obs.myhuaweicloud.com/package/ptdbg_ascend/1.0/ptdbg_ascend-1.0-py3-none-any.whl) | [工具使用指南](doc/ptdbg_ascend精度工具功能说明_v1.0.md) |
| 2.1            | 2023-4-21 |1.8.1/1.11.0| [ptdbg_ascend-2.1-py3-none-any.whl](https://ptdbg.obs.myhuaweicloud.com/package/ptdbg_ascend/2.0/ptdbg_ascend-2.1-py3-none-any.whl) | [工具使用指南](doc/ptdbg_ascend精度工具功能说明_v2.1.md) |
### 从源码安装

您可以从源代码构建 ptdbg_ascend 软件包并将其安装在带NPU或者GPU的AI计算环境上。
> ptdbg_ascend 与 Pytorch 有严格的版本配套关系，从源码构建前，您需要确保已经正确安装了[Pytorch v1.8.1 或 V1.11.0 版本](https://www.pytorch.org) 。

#### 环境和依赖
编译前需要安装wheel包
```
   pip install wheel
```

#### 下载源码

```
git clone https://gitee.com/ascend/tools.git
cd tools/ptdbg_ascend
```

#### 配置安装环境

```BASH
bash ./configure
```

默认情况下，执行上述命会弹出如下的交互式会话窗口
> 您的会话可能有所不同。

```BASH
Please specify the location of python with available pytorch v1.8.1/v1.11.0 site-packages installed. [Default is /usr/bin/python3]
(You can make this quiet by set env [ADAPTER_TARGET_PYTHON_PATH]):
```

此时，要求您输入安装了 Pytorch v1.8.1或者v1.11.0 版本的python解释器路径，如果默认路径是正确的，直接回车，否则请输入正确的 python 解释器路径。
> 您可以通过设置 ADAPTER_TARGET_PYTHON_PATH的环境变量，来抑制交互式窗口弹出，但是要确保路径是有效的，否则，仍然会要求您输入正确的 python 解释器路径。

键入后，会耗费几秒钟以确保您的输入是有效的，配置完成后会输出如下提示信息。
```BASH
Configuration finished
```

#### 配置cmake

> 根据您的网络状况，可能需要数分钟来下载ptdbg_ascend的依赖项目以完成配置。

```
mkdir build
cd build
cmake ..
```

#### 执行编译

> 您应当根据实际编译环境，设置合适的并发编译数以提升编译速度。

```BASH
make -j8
```

编译结束后，安装包会生成在

```
./ptdbg_ascend/dist/ptdbg_ascend-0.1-py3-none-any.whl
```

#### 安装

您可以继续执行

```BASH
make install
```

将ptdbg_ascend安装到配置时指定的 python 解释器包目录下，或者使用 pip3 安装 ptdbg_ascend 到您期望的位置。

```
pip3 install ./ptdbg_ascend/dist/ptdbg_ascend-0.1-py3-none-any.whl --upgrade --force-reinstall
```



## 贡献

push代码前，请务必保证已经完成了基础功能测试和网络测试！

## Release Notes

Release Notes请参考[RELEASE](RELEASE.md).
