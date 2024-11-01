# tools

#### 介绍

Ascend tools，昇腾工具仓库。

**请根据自己的需要进入对应文件夹获取工具，或者点击下面的说明链接选择需要的工具进行使用。**

**版本配套和硬件配套请参见各子文件夹的配套说明**

#### 使用说明

1.  [msame](https://gitee.com/ascend/tools/tree/master/msame)

    **模型推理工具**：输入.om模型和模型所需要的输入bin文件，输出模型的输出数据文件。

2.  [ais-bench_workload](https://gitee.com/ascend/tools/tree/master/ais-bench_workload)

    **ais-bench_workload工具**：基于AISBench测试基准的模型负载代码以及为AISBench测试基准贡献的高易用性子工具，用于AI服务器的性能测试。

3.  [opdump_compare](https://gitee.com/ascend/tools/tree/master/opdump_compare)
    
    **opdump_compare工具**：算子仿真dump数据指令序分析工具，用于分析同一算子实现在不同CANN版本下的指令序变化。

4. [precision_tool](https://gitee.com/ascend/tools/tree/master/precision_tool)    
   **精度问题分析工具**：该工具包提供了精度比对常用的功能，当前该工具主要适配Tensorflow训练场景，同时提供Dump数据/图信息的交互式查询和操作入口。 


#### 贡献

欢迎参与贡献。更多详情，请参阅我们的[贡献者Wiki](./CONTRIBUTING.md)。

#### 许可证
[Apache License 2.0](LICENSE)

