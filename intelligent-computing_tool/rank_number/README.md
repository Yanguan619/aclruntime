# rank_number（找缺失的rank_number场景小工具）

### 功能
目前日志是第一部分的，server端和client端建链超时，即预期server端与2168个client建链，实际只建链2162，在server端会打出来所有已建链的id（0~n之间依次递增，但可能有空缺数字），需要找出缺失的数字。

### 使用环境
已安装python的设备。  

### 获取
- 命令行方式下载

   **git clone https://gitee.com/ascend/tools.git**

- 压缩包方式下载

    1. tools仓右上角选择 **克隆/下载** 下拉框并选择 **下载ZIP**。

    2. 将ZIP包上传到开发环境中的普通用户家目录中，例如 **$HOME/ascend-tools-master.zip**。

    3. 开发环境中，执行以下命令，解压zip包。

        **unzip ascend-tools-master.zip**


### 使用方法
业务主要分为三步：
1、通信域初始化：即host建链，收集全卡npu IP并分发
     rank0 起server，所有卡起client
2、npu间建链，npu ip  在第一阶段已知，做npu间链路建立，并交换关键数据（dma、notify相关数据）
3、task(包括dma、notify)下发调度执行

### 参数说明

 参数名	                说明
--log_file_path	日志文件存放路径

