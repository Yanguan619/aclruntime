# 精度问题分析工具

## 版本配套   
| 条件 | 要求 | 备注 |
|---|---|---|
| CANN版本 | >=5.0.4.6| 请获取配套版本的“[CANN软件安装指南](https://www.hiascend.com/document/redirect/CannCommercialInstSoftware)”进行CANN软件安装。 |
| 硬件要求 | **支持以下产品：**<br>- Atlas训练系列产品<br>- Atlas A2训练系列产品| 支持的固件驱动版本与配套CANN软件支持的固件驱动版本相同，开发者可通过“[昇腾社区-固件与驱动](https://www.hiascend.com/hardware/firmware-drivers/community)”页面根据产品型号与CANN软件版本获取配套的固件与驱动。 |


## 功能介绍
该工具包提供了精度比对常用的功能，仅适用于Tensorflow训练场景，主要提供了如下功能：
- 浮点异常检测
- 融合异常检测
- 整网数据比对
- 随机错误检测

## 使用须知    

>  本README仅给出precision_tool工具的使用前提以及相关命令说明，关于此工具不同功能场景下详细的端到端操作可参见配套版本的[“TensorFlow模型迁移指南 > 精度调优”](https://www.hiascend.com/document/redirect/CannCommercialTfAccuracy)手册。
> 
## 工具准备

1. 工具包下载（二选一）。
   - 下载压缩包的方式获取tools仓源码。
   - 使用git命令，下载tools仓源码，命令示例：
     ```shell
     git clone https://gitee.com/ascend/tools.git
     ```
2. 移动"tools/precision_tool"子目录至TensorFlow网络训练工作目录。

## 使用前提

1. 安装python3三方依赖
   ```shell
   pip3 install rich gnureadline pexpect
   # ubuntu/Debian
   sudo apt-get install graphviz
   # fedora/Centos
   sudo yum install graphviz
   ```
2. 环境上能够正常执行CPU和NPU训练脚本。
3. 训练脚本去随机处理。
  
   如果需要进行整网数据比对（Dump数据比对），需要先检查并去除训练脚本内部使用到的随机处理，避免由于输入数据不一致导致数据比对结果不可用。
    ```python
    # 对于使用tf.random / np.random / (python) random的可以通过固定随机种子的方式固定输入
    # import tf_config.py 默认会设置上述三种random的seed，但由于import位置关系，可能不一定能作用到所有的关联代码，建议在代码确认合适位置手动嵌入
    seed =987654
    random.seed(seed)
    tf.random.set_random_seed(seed)
    np.random.seed(seed)
  
    # RunConfig/NPURunConfig中设置tf_random_seed固定网络随机因子
    # Estimator中tf.random设置的随机种子并不能全局生效
    # 需要使用下面的方式进行设置
    run_config = tf.estimator.RunConfig(tf_random_seed=1, ...)
    run_config = NPURunConfig(tf_random_seed=1, ...)
    ```
    **理论上网络中的大多数随机处理均能通过上面的方式固定, 其他情况可参考以下示例进行脚本排查（下述仅为典型示例）：**
    ```python
    # 1. 参数初始化中的随机操作
    #    加载checkpoint的方式能够固定大多数初始参数
    saver.restore(sess, saver_dir)
    
    # 2. 输入数据的随机操作（例如对输入数据做shuffle操作）
    dataset = tf.data.TFRecordDataset(tf_data)
    dataset = dataset.shuffle(batch_size*10)    # 直接注释掉该行
    
    # 3. 模型中的随机操作（例如使用dropout）
    net = slim.dropout(net, keep_prob=dropout_keep_prob, scope='Dropout_1b') # 建议注释该行
    
    # 4. 图像预处理使用的随机操作(根据实际情况固定随机种子，或者替换成其他固定的预处理操作)
    # 4.1 Random rotate
    random_angle = tf.random_uniform([], - self.degree * 3.141592 / 180, self.degree * 3.141592 / 180)
    image = tf.contrib.image.rotate(image, random_angle, interpolation='BILINEAR')
    depth_gt = tf.contrib.image.rotate(depth_gt, random_angle, interpolation='NEAREST')
  
    # 4.2 Random flipping
    do_flip = tf.random_uniform([], 0, 1)
    image = tf.cond(do_flip > 0.5, lambda: tf.image.flip_left_right(image), lambda: image)
    depth_gt = tf.cond(do_flip > 0.5, lambda: tf.image.flip_left_right(depth_gt), lambda: depth_gt)
    
    # 4.3 Random crop
    mage_depth = tf.concat([image, depth_gt], 2)
    image_depth_cropped = tf.random_crop(image_depth, [self.params.height, self.params.width, 4])
  
    # 其他......
    ```


## 配置文件说明
   
precision_tool/config.py文件支持的配置如下：

```python
# 如果需要dump特定step的数据，一般对比分析dump首层即可，即保持默认值，如需指定特定step，可修改以下配置项。若不配置TF_DUMP_STEP，采集所有迭代的dump数据
# 示例：Dump config '0|5|10'
TF_DUMP_STEP = '0' 

# 依赖Toolkit包中的atc和msaccucmp.pyc工具，配置为Toolkit包的安装目录
# 默认Toolkit安装在/usr/local/Ascend，可以不用修改。指定目录安装则需要修改
CMD_ROOT_PATH = '/usr/local/Ascend'
```

## 工具命令说明

“precision_tool”工具的启动命令如下所示：

```shell
python3 ./precision_tool/cli.py 
```
进入交互式命令行界面：
 **PrecisionTool >** 
   
支持的交互模式命令说明如下：
1. ac -l [limit_num] -c
    ```shell
    # auto check，自动化检测命令。
    # 列出Fusion信息，解析算子溢出信息。
    # -c：可选，进行全网比对。
    # -l：可选，限制输出结果的条数（overflow解析的条数等）。
    PrecisionTool > ac -c
   ╭──────────────────────────────────────────────────────────────────────────────────────────────────╮
   │ [TransData][327] trans_TransData_1170                                                            │
   │  - [AI Core][Status:32][TaskId:327] ['浮点计算有溢出']                                           │
   │  - First overflow file timestamp [1619347786532995] -                                            │
   │  |- TransData.trans_TransData_1170.327.1619347786532995.input.0.npy                              │
   │   |- [Shape: (32, 8, 8, 320)] [Dtype: bool] [Max: True] [Min: False] [Mean: 0.11950836181640626] │
   │  |- TransData.trans_TransData_1170.327.1619347786532995.output.0.npy                             │
   │   |- [Shape: (32, 20, 8, 8, 16)] [Dtype: bool] [Max: True] [Min: False] [Mean: 0.07781982421875] │
   ╰──────────────────────────────────────────────────────────────────────────────────────────────────╯
    ```
2. run [command]
    ```shell
    # 不退出交互命令环境执行shell命令，与内置命令不冲突的可以直接执行，否则需要加run前缀。
    PrecisionTool > run vim cli.py
    PrecisionTool > vim cli.py
    ```

3. ls -n [op_name] -t [op_type] -f [fusion_pass] -k [kernel_name]
    ```shell
    # 通过[算子名]/[算子类型]查询网络里的算子，模糊匹配。
    # -n：可选，算子节点名称。
    # -t：可选，算子类型。
    # -f：可选，融合类型。
    # -k：可选，kernel_name。
    # 说明：-n与-t需要存在其中一个输入。
    PrecisionTool > ls -t Mul -n mul_3 -f TbeMulti
   [Mul][TbeMultiOutputFusionPass] InceptionV3/InceptionV3/Mixed_5b/Branch_1/mul_3
   [Mul][TbeMultiOutputFusionPass] InceptionV3/InceptionV3/Mixed_5c/Branch_1/mul_3
   [Mul][TbeMultiOutputFusionPass] InceptionV3/InceptionV3/Mixed_5d/Branch_1/mul_3
   [Mul][TbeMultiOutputFusionPass] InceptionV3/InceptionV3/Mixed_6b/Branch_1/mul_3
    ```

4. ni (-n) [op_name] -s [save sub graph deep]
    ```shell
    # 通过[算子名]查询算子节点信息。
    # -n：可选，指定节点名称。
    # -g：可选，graph名称。
    # -a：可选，显示attr信息。
    # -s：可选，保存一个以当前算子节点为根，深度为参数值的子图。
    # 说明：-n与-g需要存在其中一个输入。
   PrecisionTool >  ni gradients/InceptionV3/InceptionV3/Mixed_7a/Branch_0/Maximum_1_grad/GreaterEqual -s 3
   ╭─────────────────── [GreaterEqual]gradients/InceptionV3/InceptionV3/Mixed_7a/Branch_0/Maximum_1_grad/GreaterEqual ────────────────────╮
   │ [GreaterEqual] gradients/InceptionV3/InceptionV3/Mixed_7a/Branch_0/Maximum_1_grad/GreaterEqual                                       │
   │ Input:                                                                                                                               │
   │  -[0][DT_FLOAT][NHWC][32, 8, 8, 320] InceptionV3/InceptionV3/Mixed_7a/Branch_0/add_3:0                                               │
   │  -[1][DT_FLOAT][NHWC][1, 8, 1, 1] InceptionV3/Mixed_7a/Branch_0/Conv2d_1a_3x3tau:0                                                   │
   │  -[2][][[]][] atomic_addr_clean0_21:-1                                                                                               │
   │ Output:                                                                                                                              │
   │  -[0][DT_BOOL][NHWC][32, 8, 8, 320] ['trans_TransData_1170']                                                                         │
   │ NpuDumpInput:                                                                                                                        │
   │  -[0] GreaterEqual.gradients_InceptionV3_InceptionV3_Mixed_7a_Branch_0_Maximum_1_grad_GreaterEqual.325.1619494134722860.input.0.npy  │
   │   |- [Shape: (32, 8, 8, 320)] [Dtype: float32] [Max: 5.846897] [Min: -8.368301] [Mean: -0.72565556]                                  │
   │  -[1] GreaterEqual.gradients_InceptionV3_InceptionV3_Mixed_7a_Branch_0_Maximum_1_grad_GreaterEqual.325.1619494134722860.input.1.npy  │
   │   |- [Shape: (1, 8, 1, 1)] [Dtype: float32] [Max: 0.0] [Min: 0.0] [Mean: 0.0]                                                        │
   │ NpuDumpOutput:                                                                                                                       │
   │  -[0] GreaterEqual.gradients_InceptionV3_InceptionV3_Mixed_7a_Branch_0_Maximum_1_grad_GreaterEqual.325.1619494134722860.output.0.npy │
   │   |- [Shape: (32, 8, 8, 320)] [Dtype: bool] [Max: True] [Min: False] [Mean: 0.1176300048828125]                                      │
   │ CpuDumpOutput:                                                                                                                       │
   │  -[0] gradients_InceptionV3_InceptionV3_Mixed_7a_Branch_0_Maximum_1_grad_GreaterEqual.0.1619492699305998.npy                         │
   │   |- [Shape: (32, 8, 8, 320)] [Dtype: bool] [Max: True] [Min: False] [Mean: 0.11764373779296874]                                     │
   ╰──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
   2021-04-27 14:39:55 (15178) -[DEBUG]write 14953 bytes to './precision_data/dump/temp/op_graph/GreaterEqual.gradients_InceptionV3_InceptionV3_Mixed_7a_Branch_0_Maximum_1_grad_GreaterEqual.3.gv'
   2021-04-27 14:39:55 (15178) -[INFO]Sub graph saved to /root/sym/inception/precision_data/dump/temp/op_graph
   ```
   
5. pt (-n) [*.npy] 
    ```shell
    # 查看某个dump数据块的数据信息，并保存到txt文件。
    # -n：可选，待查看的数据文件名。
    PrecisionTool > pt TransData.trans_TransData_1170.327.1619347786532995.input.0.npy 
   ╭─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
   │ Shape: (32, 8, 8, 320)                                                                                                  │
   │ Dtype: bool                                                                                                             │
   │ Max: True                                                                                                               │
   │ Min: False                                                                                                              │
   │ Mean: 0.11950836181640626                                                                                               │
   │ Path: ./precision_data/dump/temp/overflow_decode/TransData.trans_TransData_1170.327.1619347786532995.input.0.npy        │
   │ TxtFile: ./precision_data/dump/temp/overflow_decode/TransData.trans_TransData_1170.327.1619347786532995.input.0.npy.txt │
   ╰─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
    ```

6. cp (-n) [left *.npy] [right *.npy] -p [print num] -al [atol] -rl [rtol]
    ```shell
    # 对比两个numpy文件中的tensor数据。
    # -n：必选，指定需要对比的两个numpy文件的文件名。
    # -p：可选，指定输出前多少个错误数据。
    # -al/rl：可选，al为绝对误差，rl为相对误差，使用示例如下：
    #   示例1. np.allclose(left, right, atol=al, rtol=rl)
    #   示例2. err_cnt += 1 if abs(data_left[i] - data_right[i]) > (al + rl * abs(data_right[i]))
    # -s：可选，保存成txt文件，默认打开
    PrecisionTool > cp Add.InceptionV3_InceptionV3_Mixed_7a_Branch_0_add_3.323.1619494134703053.output.0.npy InceptionV3_InceptionV3_Mixed_7a_Branch_0_add_3.0.1619492699305998.npy -p 10 -s -al 0.002 -rl 0.005
                      Error Item Table                                        Top Item Table
   ┏━━━━━━━┳━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━┓ ┏━━━━━━━┳━━━━━━━━━━━━━┳━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━┓
   ┃ Index ┃ Left          ┃ Right        ┃ Diff         ┃ ┃ Index ┃ Left        ┃ Right       ┃ Diff          ┃
   ┡━━━━━━━╇━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━┩ ┡━━━━━━━╇━━━━━━━━━━━━━╇━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━┩
   │ 155   │ 0.024600908   │ 0.022271132  │ 0.002329776  │ │ 0     │ -0.9206961  │ -0.9222216  │ 0.0015255213  │
   │ 247   │ 0.015752593   │ 0.017937578  │ 0.0021849852 │ │ 1     │ -0.6416973  │ -0.64051837 │ 0.0011789203  │
   │ 282   │ -0.0101207765 │ -0.007852031 │ 0.0022687456 │ │ 2     │ -0.35383835 │ -0.35433492 │ 0.0004965663  │
   │ 292   │ 0.019581757   │ 0.02240482   │ 0.0028230622 │ │ 3     │ -0.18851271 │ -0.18883198 │ 0.00031927228 │
   │ 640   │ -0.06593232   │ -0.06874806  │ 0.0028157383 │ │ 4     │ -0.43508735 │ -0.43534422 │ 0.00025686622 │
   │ 1420  │ 0.09293677    │ 0.09586689   │ 0.0029301196 │ │ 5     │ 1.4447614   │ 1.4466647   │ 0.0019032955  │
   │ 1462  │ -0.085207745  │ -0.088047795 │ 0.0028400496 │ │ 6     │ -0.3455438  │ -0.3444429  │ 0.0011008978  │
   │ 1891  │ -0.03433288   │ -0.036525503 │ 0.002192624  │ │ 7     │ -0.6560242  │ -0.6564579  │ 0.0004336834  │
   │ 2033  │ 0.06828873    │ 0.07139922   │ 0.0031104907 │ │ 8     │ -2.6964858  │ -2.6975214  │ 0.0010356903  │
   │ 2246  │ -0.06376442   │ -0.06121233  │ 0.002552092  │ │ 9     │ -0.73746175 │ -0.73650354 │ 0.00095820427 │
   └───────┴───────────────┴──────────────┴──────────────┘ └───────┴─────────────┴─────────────┴───────────────┘
   ╭──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
   │ Left:                                                                                                                                    │
   │  |- NpyFile: ./precision_data/dump/temp/decode/Add.InceptionV3_InceptionV3_Mixed_7a_Branch_0_add_3.323.1619494134703053.output.0.npy     │
   │  |- TxtFile: ./precision_data/dump/temp/decode/Add.InceptionV3_InceptionV3_Mixed_7a_Branch_0_add_3.323.1619494134703053.output.0.npy.txt │
   │  |- NpySpec: [Shape: (32, 8, 8, 320)] [Dtype: float32] [Max: 5.846897] [Min: -8.368301] [Mean: -0.72565556]                              │
   │ DstFile:                                                                                                                                 │
   │  |- NpyFile: ./precision_data/dump/cpu/InceptionV3_InceptionV3_Mixed_7a_Branch_0_add_3.0.1619492699305998.npy                            │
   │  |- TxtFile: ./precision_data/dump/cpu/InceptionV3_InceptionV3_Mixed_7a_Branch_0_add_3.0.1619492699305998.npy.txt                        │
   │  |- NpySpec: [Shape: (32, 8, 8, 320)] [Dtype: float32] [Max: 5.8425903] [Min: -8.374472] [Mean: -0.7256237]                              │
   │ NumCnt:   655360                                                                                                                         │
   │ AllClose: False                                                                                                                          │
   │ CosSim:   0.99999493                                                                                                                     │
   │ ErrorPer: 0.023504638671875  (rl= 0.005, al= 0.002)                                                                                      │
   ╰──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
    ```

7. vc -lt [left_path] -rt [right_path] -g [graph]
   ```shell
    # 用于手动指定两个目录进行整网精度对比
    # -lt：必选，其中一个文件目录。
    # -rt：必选，另一个目录，一般是标杆目录。
    # 需要指定到dump数据所在的目录层级，precision_data/npu/debug_0/dump/20220217095546/3/ge_default_20220217095547_1/1/0/
    # -g：可选，指定-g将尝试解析graph内的映射关系比对（一般用于NPU和TensorFlow之间的数据比对， NPU与NPU之间比对不需要，直接按照算子name对比）
   ```
8. vcs -f [file_name] -c [cos_sim_threshold] -l [limit]
   ```shell
    # 查看精度比对结果的概要信息，可以更加预先相似的阈值过滤出低于阈值的算子/信息。
    # -f (--file)：可选，指定csv文件，不设置则默认遍历precision_data/temp/vector_compare/目录下最近产生的对比目录内的所有csv。
    # -c (--cos_sim)：可选，指定筛选所使用的预先相似度阈值，默认0.98。
    # -l (--limit)：可选，指定输出前多少个结果，默认值3。
    PrecisionTool > vcs -c 0.98 -l 2
    2021-05-31 14:48:56 (2344298) -[INFO]Sub path num:[1]. Dirs[['20210529145750']], choose[20210529145750]
    2021-05-31 14:48:56 (2344298) -[DEBUG]Find ['result_20210529145751.csv', 'result_20210529145836.csv', 'result_20210529145837.csv', 'result_20210529145849.csv', 'result_20210529150404.csv', 'result_20210529151102.csv'] result files in dir precision_data/temp/vector_compare/20210529145750
    2021-05-31 14:48:56 (2344298) -[INFO]Find 0 ops less then 0.98 in precision_data/temp/vector_compare/20210529145750/result_20210529145751.csv
    2021-05-31 14:48:56 (2344298) -[INFO]Find 0 ops less then 0.98 in precision_data/temp/vector_compare/20210529145750/result_20210529145836.csv
    2021-05-31 14:48:56 (2344298) -[INFO]Find 1 ops less then 0.98 in precision_data/temp/vector_compare/20210529145750/result_20210529145837.csv
    2021-05-31 14:48:56 (2344298) -[INFO]Find 2 ops less then 0.98 in precision_data/temp/vector_compare/20210529145750/result_20210529145849.csv
    2021-05-31 14:48:56 (2344298) -[INFO]Find 2 ops less then 0.98 in precision_data/temp/vector_compare/20210529145750/result_20210529150404.csv
    2021-05-31 14:48:56 (2344298) -[INFO]Find 0 ops less then 0.98 in precision_data/temp/vector_compare/20210529145750/result_20210529151102.csv
    ╭── [578] pixel_cls_loss/cond_1/TopKV2 ───╮
    │ Left:  ['pixel_cls_loss/cond_1/TopKV2'] │
    │ Right: ['pixel_cls_loss/cond_1/TopKV2'] │
    │ Input:                                  │
    │  - [0]1.0        - [1]nan               │
    │ Output:                                 │
    │  - [0]0.999999   - [1]0.978459          │
    ╰─────────────────────────────────────────╯
    ╭── [490] gradients/AddN_5 ───╮
    │ Left:  ['gradients/AddN_5'] │
    │ Right: ['gradients/AddN_5'] │
    │ Input:                      │
    │  - [0]nan        - [1]1.0   │
    │ Output:                     │
    │  - [0]0.05469               │
    ╰─────────────────────────────╯
   ```
## Precision_data目录结构介绍
示例如下：
```
precision_data/
├── npu
│   ├── debug_0
|   |   ├── dump
|   |       └── 20210510101133
|   │   └── graph
|   |       └── ge_proto_00000179_PreRunAfterBuild.txt
│   └── debug_1
├── tf
|   ├── tf_debug
|   └── dump
├── overflow
├── fusion
└── temp
    ├── op_graph
    ├── decode
    |   ├── dump_decode
    |   ├── overflow_decode
    |   └── dump_convert
    └── vector_compare
        ├── 20210510101133
        |   ├── result_123456.csv
        |   └── result_123455.csv
        └── 20210510101134
            └── result_123458.csv
```

## 常见问题处理
1. 安装gnureadline报错找不到lncurses
   
   错误信息如下：
   ```shell
   /usr/bin/ld: cannot find -lncurses
   collect2: error: ld returned 1 exit status
   error: command 'gcc' failed with exit status 1
   ```
   解决方法如下：
   ```shell
   # 先尝试在本地查找libncurses.so*
   find / -name libncurses.so*
   # 如果能找到以下文件，直接创建一个libncurses.so指向libncurses.so.5即可，否则需要用包管理工具安装ncurses
   /usr/lib64/libncurses.so.5
   /usr/lib64/libncurses.so.5.9
   /usr/lib64/libncursesw.so.5
   # 创建软连接
   ln -s /usr/lib64/libncurses.so.5.9 /usr/lib64/libncurses.so
   ```
