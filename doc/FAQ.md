# FAQ

## 1. input文件夹输入有很多的数据，如果选择其中某一部分做输入进行推理。比如 input文件夹中有50000张图片，如果只选择其中100张进行推理
----------------------------------------
当前推理工具针对input文件夹中数据是全部读取的，没有读取某部分数据功能

如果需要该功能，可以通过如下脚本命令执行，生成某一部分的软链接的文件夹，传入到推理程序中。

```bash
# 首先搜索src目录下的所有的JPEG的文件  然后选取前100个 然后通过软链接的方式链接dst文件夹中
find ./src -type f -name "*.JPEG" | head -n 100 | xargs -i ln -sf {} ./dst
```

## 2. 推理工具运行时，会出现aclruntime版本不匹配告警
**故障现象**
运行推理工具进行推理时屏幕输出如下告警：
```bash
root#  python3 -m ais_bench --model /home/lhb/code/testdata/resnet50/model/pth_resnet50_bs1.om --loop 2
[WARNING] aclruntime version:0.0.1 is lower please update aclruntime follow any one method
[WARNING] 1. visit https://gitee.com/ascend/tools/tree/master/ais-bench_workload/tool/ais_bench to install
[WARNING] 2. or run cmd: pip3  install -v --force-reinstall 'git+https://gitee.com/ascend/tools.git#egg=aclruntime&subdirectory=ais-bench_workload/tool/ais_bench/aclruntime' to install
```
**故障原因：**
环境安装低版本aclruntime, 推理工具运行时使用的是高版本的ais_bench

**处理步骤：**
更新aclruntime程序包
## 3. 推理工具组合输入进行推理时遇到"save out files error"

总结：

save out files error错误是在推理输出结果进行切分函数中发生的，推理的输出结果会根据输出文件信息进行切分，在切分过程中执行失败。主要的错误原因有如下两个，一种是切分的batch轴不是最高维度0，如故障现象1；另一种是输入文件大小不对应，导致切分失败，

出现该问题请开启debug模式，仔细检查模型输入信息和文件输入信息和shape信息。检查为什么切分失败。

**故障现象1**

```bash
[ERROR] save out files error array shape:(1, 349184, 2) filesinfo:[['prep/2002_07_19_big_img_18.bin', 'prep/2002_07_19_big_img_90.bin', 'prep/  2002_07_19_big_img_130.bin', 'prep/2002_07_19_big_img_135.bin', 'prep/  2002_07_19_big_img_141.bin', 'prep/2002_07_19_big_img_158.bin', 'prep/  2002_07_19_big_img_160.bin', 'prep/2002_07_19_big_img_198.bin', 'prep/  2002_07_19_big_img_209.bin', 'prep/2002_07_19_big_img_230.bin', 'prep/  2002_07_19_big_img_247.bin', 'prep/2002_07_19_big_img_254.bin', 'prep/  2002_07_19_big_img_255.bin', 'prep/2002_07_19_big_img_269.bin', 'prep/  2002_07_19_big_img_278.bin', 'prep/2002_07_19_big_img_300.bin']]  files_count_perbatch:16 ndata.shape0:1
```
**故障原因**
input文件由16个文件组成，推理输出进行结果文件切分时，默认按shape的第一维切分，而shape最高维度是1，不是16的倍数。所以报错
**处理步骤**
推理工具参数"--output_batchsize_axis"取值为1。 改成以shape第2维进行切分

**故障现象2**

```
[ERROR] save out files error array shape:(1, 256, 28, 28) filesinfo:[['dump_data_npu/group_in.npy', 'padding_infer_fake_file']] files_count_perbatch:2 ndata.shape0:1
```

**故障原因**
从错误打印上分析，推理的输入文件有两个，第二个文件padding_infer_fake_file是为了补齐长度的构造的文件，出现这个文件的情况是因为输入文件的大小与模型的对应输入大小不相等，是倍数关系，本例中是2倍的关系，也就说模型输入的大小是输入文件大小的两倍，所以会增加一个构造文件。

本打印的函数是根据模型文件个数切分模型的输出，默认按照输出shape的最高维进行切分，因为最高 维度为1，切分不了2，所以报错了。

**处理步骤**
多个输入文件进行组batch进行推理主要是针对一些batch大于1的模型。比如batch=2的模型，那么2个输入的文件组合在一起进行推理。但是本样例场景下，输入文件的数据是模型需要的一半大小。所以自动进行组batch，并且增加了一个padding_infer_fake_file文件，但其实是用户的输入文件大小不应该输入一半，输入错了。因为该模型不是多batch的模型。

将输入文件大小修改为模型输入大小。问题解决

## 4. acl open device 0 failed推理npu设备打开失败
**故障现象**
```bash
python3 -m ais_bench --model ./bert_base_chanese_bs32.om --device 0
[INFO] acl init success
EL003: The argument is invalid.
       ...
	   Failed to open device,retCode=0x7020014,deviceId=0.[FUNC:Init][FILE:device.cc][LINE:211]
	   ...
	   open device 0 failed,runtime result = 507033.[FUNC:ReportCallError][FILE:log_inner.cpp][LINE：162]
[ERROR] acl open device 0 failed
SetDevice failed.ret=507033
```
**分析：**
acl open device 0 failed一般是因为驱动和固件安装或者硬件有问题，请排查驱动和固件或者硬件，看下是否安装正常**

## 5. tensorsize与filesize 不匹配推理失败
**故障现象**

```bash
python3 -m ais_bench --model ./testdata/resnet50/model/pth_resnet50_bs1.om --input ./testdata/resnet50/input/602112/602112.bin
[INFO] try get model batchsize:1
[ERROR] arg0 tensorsize: 196608 filesize: 602112 not match
```
**故障原因：**

出现该问题主要是因为模型输入大小与文件大小不匹配导致。

模型pth_resnet50_bs1.om输入actual_input_1为1*3*256*256，合计196608字节大小。但input参数602112.bin大小为602112。与om文件输入要求不匹配
**处理步骤：**

请查看模型输入需要大小。将文件输入文件大小调整为对应大小。

本例中更换input参数对象为196608字节大小的文件  即可解决问题。

## 6. 推理命令执行正常，增加profiler参数使能profiler功能时出现报错，提示推理命令中的路径找不到
**故障现象**

```bash
# 基础推理命令执行正常
$ python3 -m ais_bench --model=search_bs1.om --output ./

# 在基础推理命令上增加profiler参数使能，报错，提示模型路径找不到
$ python3 -m ais_bench --model=search_bs1.om --output ./ --profiler 1
...
[ERROR] load model from file failed, model file is search_bs1.om
...
RuntimeError:[1][ACL:invalid parameter]

# 基础命令中执行成功
$ python3 -m ais_bench --model=/home/search_bs1.om --output ./  --input ./1.bin,./2.bin
# 基础命令中增加profiler参数使能，报错，提示文件输入路径不存在
$ python3 -m ais_bench --model=/home/search_bs1.om --output ./  --input ./1.bin,./2.bin --profiler 1
...
[ERROR] Invalid args. ./1.bin of --input is invalid
```
**故障原因：**
	出现该问题原因是因为使能profiler功能后，相对路径被profiler模块解析时会有些问题，导致运行目录切换，相对路径找不到，当前版本暂未修复。

**处理步骤：**

​	出现该问题，请将model  input output等参数里的相对路径修改为绝对路径，这样可以临时规避。

## 使用--acl_json_path参数无法dump数据

```log
root@davinci-mini:/data/workspace# python3.11 -m ais_bench --model vision_encoder.om --input input_0.bin --device 0 --output dump_data/npu --warmup_count 0 --acl_json_path 20260706095100/dump_data/npu/acl.json
[INFO] acl init success
[INFO] TDT(52728,python3.11):2026-07-06-10:33:04.735.072 [client_manager.cpp:462][GetClientRunMode][tid:52728] runningMode:0
[INFO] TDT(52728,python3.11):2026-07-06-10:33:04.735.160 [client_manager.cpp:126][GetInstance][tid:52728] [ClientManager] Current mode:2
[INFO] TDT(52728,python3.11):2026-07-06-10:33:04.735.206 [thread_mode_manager.cpp:70][Open][tid:52728] [ThreadModeManager] enter into open process deviceId[0] rankSize[0]
[INFO] TDT(52728,python3.11):2026-07-06-10:33:04.736.448 [thread_mode_manager.cpp:280][HandleAICPUPackage][tid:52728] begin load aicpu package dstPath[/root/], srcpath[/usr/local/Ascend/cann-9.0.T3/opp/Ascend310P/aicpu/] file[Ascend310P-aicpu_syskernels.tar.gz]
[INFO] TDT(52728,python3.11):2026-07-06-10:33:04.736.527 [package_worker.cpp:337][LoadAICPUPackageForThreadMode][tid:52728] Package checkcode is [52605468]
[WARNING] TDT(52728,python3.11):2026-07-06-10:33:04.736.587 [package_worker.cpp:341][LoadAICPUPackageForThreadMode][tid:52728] Open aicpu_package_install.info verifyFile[/root/aicpu_package_install.info], strerror[File exists]
[INFO] TDT(52728,python3.11):2026-07-06-10:33:04.736.903 [thread_mode_manager.cpp:280][HandleAICPUPackage][tid:52728] begin load aicpu package dstPath[/root/], srcpath[/usr/local/Ascend/cann-9.0.T3/opp/Ascend310P/aicpu/] file[Ascend310P-aicpu_extend_syskernels.tar.gz]
[INFO] TDT(52728,python3.11):2026-07-06-10:33:04.736.963 [package_worker.cpp:337][LoadAICPUPackageForThreadMode][tid:52728] Package checkcode is [10729130]
[WARNING] TDT(52728,python3.11):2026-07-06-10:33:04.737.012 [package_worker.cpp:341][LoadAICPUPackageForThreadMode][tid:52728] Open aicpu_package_install.info verifyFile[/root/extend_aicpu_package_install.info], strerror[File exists]
[INFO] TDT(52728,python3.11):2026-07-06-10:33:04.737.093 [thread_mode_manager.cpp:56][OpenTfSo][tid:52728] access tf path /root/aicpu_kernels/0/aicpu_kernels_device/sand_box/ abnormal:No such file or directory.
[WARNING] TDT(52728,python3.11):2026-07-06-10:33:04.737.194 [thread_mode_manager.cpp:115][StartCallAICPU][tid:52728] [ThreadModeManager] Can not open libaicpu_scheduler.so, deviceId[0], reason[/usr/lib64/libaicpu_scheduler.so: cannot open shared object file: No such file or directory]
[INFO] TDT(52728,python3.11):2026-07-06-10:33:04.741.738 [thread_mode_manager.cpp:159][SetAICPUProfilingCallback][tid:52728] [ThreadModeManager] profiling callback is nullptr, skip set aicpu profiling callback
[INFO] TDT(52728,python3.11):2026-07-06-10:33:04.776.025 [client_manager.cpp:195][SetProfilingCallback][tid:52728] [TsdClient] set profiling callback success
[INFO] open device 0 success
[INFO] create new context
[ACL ERROR] E39999: Inner Error!
E39999[PID: 52728] 2026-07-06-10:33:04.048.187 (E39999):  No TsdCloseEx symbol found in libtsdclient.so.[FUNC:TsdClientInit][FILE:runtime.cc][LINE:318]
        TraceBack (most recent call last):
       Failed to synchronize DataDumpLoadInfoTask, retCode=0x7150028[FUNC:DatadumpInfoLoad][FILE:context.cc][LINE:1710]
       rtDatadumpInfoLoadWithFlag execution failed, reason=aicpu timeout[FUNC:FuncErrorReason][FILE:error_message_manage.cc][LINE:65]
       Call rtDatadumpInfoLoad failed, length:213000, ret:507017[FUNC:ExecuteLoadDumpInfo][FILE:data_dumper.cc][LINE:868]
       [Model][FromData]load model from data failed, ge result[507017][FUNC:ReportCallError][FILE:log_inner.cpp][LINE:148]
```
