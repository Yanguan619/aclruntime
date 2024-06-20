# 工具介绍<a name="ZH-CN_TOPIC_0000001204082883"></a>

## 适用场景<a name="section195211910162419"></a>

此工具主要是为了对比图或者profiling序列的topo差异，例如：在图模式下，原始图和编译后的图或者对比图模式和单算子模式下的执行序列的topo差异

## 环境准备<a name="section070214015252"></a>

使用比对工具前，请确保环境已安装以下模块：

1.  已安装python软件，推荐使用python3版本。

    ubuntu系统下可使用apt-get install python3 命令安装。

## 工具获取途径<a name="section1265610537259"></a>

图对比工具获取路径：
https://gitee.com/ascend/tools/tree/master/compare_topo_order

# 工具使用<a name="ZH-CN_TOPIC_0000001158843020"></a>

**命令行格式：**  

python3 ./compare\_topo\_order.py --origin_graph=ge_proto_xxxx.txt --compiled_graph=ge_proto_xxx.txt

python3 ./compare\_topo\_order.py --single_op=kernel_detail_single_op.csv --graph_execute=kernel_detail_graph.csv

**参数说明：**

-   origin_graph:  CANN框架执行训练或atc模型转换时生成的dump图数据，可以将origin_graph选取生成的第一张图，名称里带“RunCustomPassBegin”关键字的txt格式dump图传入。**origin_graph跟single_op二选一**。

    > **说明：**
    >-   设置环境变量export DUMP\_GE\_GRAPH=2后执行训练或atc模型转换任务，会在执行文件夹下生成dump图数据。

-   compiled_graph:  CANN框架执行训练或atc模型转换时生成的dump图数据，可以将compiled_graph选取生成的最后一张图，名称里带“PreRunAfterBuild”关键字的txt格式dump图传入。**compiled_graph跟graph_execute二选一**。
-   single_op:  CANN框架执行训练或者推理时，打开profiling功能落盘的数据，将profiling数据中的xxx_ascend_pt/ASCEND_PROFILER_OUTPUT/kernel_detail.csv文件路径传入，single_op选项传入单算子模式下的profiling数据文件。**origin_graph跟single_op二选一**。
-   graph_execute:  CANN框架执行训练或者推理时，打开profiling功能落盘的数据，将profiling数据中的xxx_ascend_pt/ASCEND_PROFILER_OUTPUT/kernel_detail.csv文件路径传入，graph_execute选项传入图模式下的profiling数据文件。**compiled_graph跟graph_execute二选一**。

### 屏显结果分析<a name="section459963816435"></a>

脚本执行过程中，会有不同的日志输出，主要输出的日志行内容含义如下：

1.  topo same node num: xxx

    此日志打印表示当前对比两个图或者profiling序列，类型相同的算子个数
    
    same per: xxxx

    此日志打印表示当前对比两个图或者profiling序列，类型相同的算子个数占算子总个数的比率，即一致性

2. topo sim node num: xxx
   
    此日志打印表示当前对比两个图或者profiling序列，类型相近的算子个数（比如Matmul与MatmulV2）

    sim per: xxx

    此日志打印表示当前对比两个图或者profiling序列，类型相近的算子个数占算子总个数的比率，即相似度

### 文件落盘<a name="section459963816435"></a>

此外，此工具还会在当前脚本目录下落盘详细的比对信息
此比对信息会落盘在./compare_result目录下面

总共6个文件：

origin_graph_type.txt/single_op_type.txt
带type后缀的txt文件中记录下了原始图或者单算子profiling序列的算子类型信息

origin_graph_name.txt/single_op_name.txt
带name后缀的txt文件中记录下了原始图或者单算子profiling序列的算子名字信息

compiled_graph_type.txt/graph_execute_type.txt
带type后缀的txt文件中记录下了编译图或者图模式下profiling序列的算子类型信息

compiled_graph_name.txt/graph_execute_name.txt
带name后缀的txt文件中记录下了编译图或者图模式下profiling序列的算子名字信息

compare_result_sim.txt
记录下相似度差异以及相似度数据

compare_result_same.txt
记录下一致性差异以及一致性数据

### 待完善 <a name="section459963816435"></a>
1. 当前topo序对比工具只是单纯的对比两个图或者序列的差异，如果出现融合算子的话，无法对比融合算子内部的差异。
2. 当前相似度的映射表尚不完善，后续针对具体网络进行补全。
