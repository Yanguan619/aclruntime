# 符号化推导结果标记工具

### 功能
输入一个符号化推导结果的dump图，输出带有符号化推导结果标记的dump图，配合使用修改过的netron打开，可以通过节点颜色区分算子的符号化结果，从而识别符号化推导在那个节点中断，更快的识别符号化推导的相关问题。

绿色：符号化推导成功，节点有符号化的输出shape。

黄色：符号化推导中断，节点有符号化输入，但是没有符号化输出。

红色：未进行符号化推导，节点既没有符号化输入，也没有符号化输出。

### 获取
- 命令行方式下载

   **git clone https://gitee.com/ascend/tools.git**

- 压缩包方式下载

    1. tools仓右上角选择 **克隆/下载** 下拉框并选择 **下载ZIP**。

    2. 将ZIP包上传到开发环境中的普通用户家目录中，例如 **$HOME/ascend-tools-master.zip**。

    3. 开发环境中，执行以下命令，解压zip包。

        **unzip ascend-tools-master.zip**

### 使用方法

1. 安装依赖
    - Python3 >= 3.7.5

    - regex

        该工具依赖python开源组件regex。
        ```shell
        pip install regex
        ```
2. 执行mark_symbolize_result脚本

    脚本的输入为进行符号化推导后的dump图，请使用`BeforeAutoFuse`阶段的dump图，dump图前缀为ge_onnx，例如：`ge_onnx_00000178_graph_11_AutoFuser_BeforeAutoFuse.pbtxt`。

        ```shell
        python mark_symbolize_result <dump图文件路径>
        ```
    脚本执行成功后会在目录生成一个以_mark_symbol_infer为结尾的dump图。

3. 使用修改后的netron打开dump图的处理结果，查看符号化推导情况
    - 解压netron.tar.gz
    - 运行netron
        进入解压后的目录，运行package.py脚本。
        ```shell
        python package.py build start
        ```
    - 打开模型查看符号化推导结果
        在浏览器中打开网址`(http://localhost:8080)`,将处理后的dump图拖入网页中或者使用`Open Model`按钮打开dump图，即可查看符号化推导结果。
    - 关闭netron
       关闭网页，使用`Ctrl + C`终止`package.py`进程。
