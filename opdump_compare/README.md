# opdump_compare（算子dump数据指令序分析工具）
## 介绍  

[Tools](https://gitee.com/ascend/tools.git)仓opdump_compare目录为算子仿真dump数据指令序提取工具，用于分析同一算子实现在不同CANN版本下的指令序变化。


### 版本配套   
 
请检查以下条件要求是否满足，如不满足请按照备注进行相应处理。如果CANN版本升级，请同步检查第三方依赖是否需要重新安装）。

| 条件 | 要求                     | 备注 |
| --- |------------------------| --- |
| CANN版本 | \>=8.0.RC2             | 请参考CANN样例仓介绍中的[安装步骤](https://gitee.com/ascend/samples#%E5%AE%89%E8%A3%85)完成CANN安装，如果CANN低于要求版本请根据[版本说明](https://gitee.com/ascend/samples/blob/master/README_CN.md#%E7%89%88%E6%9C%AC%E8%AF%B4%E6%98%8E)切换samples仓到对应CANN版本 |
| 硬件要求 | Atlas 900A2/Atlas 800T | 当前已在Atlas900A2和Atlas800T 测试通过，产品说明请参考[硬件平台](https://ascend.huawei.com/zh/#/hardware/product) ，其他产品可能需要另做适配 |
  
备注1：固件与驱动版本与CANN版本的配套关系可在[下载固件与驱动](https://www.hiascend.com/hardware/firmware-drivers/community?product=1&model=30&cann=8.0.RC3.alpha003&driver=1.0.26.alpha)界面查看。

### 功能
入参input输入为仿真器dump文件路径，output为输出路径

### 使用环境
已安装开发运行环境。  

### 获取
- 命令行方式下载

   **git clone https://gitee.com/ascend/tools.git**

- 压缩包方式下载

    1. tools仓右上角选择 **克隆/下载** 下拉框并选择 **下载ZIP**。

    2. 将ZIP包上传到开发环境中的普通用户家目录中，例如 **$HOME/ascend-tools-master.zip**。

    3. 开发环境中，执行以下命令，解压zip包。

        **unzip ascend-tools-master.zip**


### 使用方法
进入opdump_compare目录
```
cd $HOME/AscendProjects/tools/opdump_compare/
```
运行脚本
```
python3 dump_log_parser.py --input xxx/xxx_instr_poped_log.dump --output ./out
```
--input 为input输入dump文件夹路径

--output 为输出output文件夹路径

会在output目录下生成每个Pipe执行的指令序文件，用于后续分析

## 输出目录示例
```
└──20241015085641
   ├── ALL.txt
   ├── CUBE.txt  
   ├── FC.txt
   ├── FIXP.txt  
   ├── FULL_ORDER.txt
   ├── MTE1.txt
   ├── MTE2.txt
   ├── MTE3.txt
   ├── SCALAR.txt
   └── VEC.txt
```

  
其他参数详情可使用--help查询。


## 注意事项
1. 运行工具的用户在当前目录需要有创建目录以及执行工具的权限，使用前请自行检查。  
2. 需要用户提前获取到仿真dump日志文件

## 参数说明

| 参数名   | 说明          |
| -------- |-------------|
| --input  | 输入dump文件夹路径 |
| --output | 输出文件夹路径     |
| --help| 工具使用帮助信息    |
