# multi_devices_perftest工具使用指南

## 工具的编译
工具的编译可以通过链接 [非执行机编译](https://aisbench.obs.cn-north-4.myhuaweicloud.com/packet/%E9%9D%9E%E6%89%A7%E8%A1%8C%E6%9C%BA%E7%BC%96%E8%AF%91.zip)中获取执行文件或者通过对应的操作系统选择对应的文件进行编译。

工具的编译只需要将依赖的so和头文件按照合适的结构进行分布： 执行机上编译需要在multi_devices_perftest下新建两个文件夹include与lib64，
include存放libibverbs提供的头文件
，lib64存放依赖的so（要区分x86和aarch64版本）


提示：理论上似乎无论是linux还是aarch64上编译好的包都可以在npu中进行测试，
但目前因为环境限制还未实测

在src下面的client/server中执行make命令既可以生成执行文件，拷贝到对应的执行机上执行即可（这里需要按照makefile提前装配好所需要的库）



## 工具使用前准备
工具使用前需要先登录虚拟网口，这里请在专业的人指导下完成，
登入虚拟网口后，则将准备好的工具包ibv_client与ibv_server传入到虚拟网口上即可。



## 工具的编译


**参数介绍**


```
-q, client 到每个 server 建立的连接数量，默认为1
-p, port 端口号，请保正client端与server端一致，默认为18515
-a, size_begin, 开始发送的 size，默认为512 KB
-b, size_end, 发送的最大的大小size，默认为128M, 为了防止可能的缓存未命中问题，实际最终发送数据为size/2，即64M
-n, iteration 每个 size 运行的轮次，默认为5000
-M, 选择push/pull 操作，默认为pull，可选参数pull，push，all（all即是可以执行完pull后再执行push）
-d, use IB device <dev> (default first device found)，一般使用hns_0（必选）
-g, gid-idx local port gid index，一般使用3或者5（必选）
-r, rx-depth number of receives to post at a time (default 500)

```
**工具的命令**

1对1场景下工具只需要两条命令，打开server端与client端即可。

Push操作：

server端：优先打开server端，这里会打印出每个server端的gid
```
./ibv_server -d hns_0 -g 3 -M push
```

client端：需要在结尾加上每个server端的GID，
其他参数与server保持一致（以逗号分隔，
这里为每个server端的gid,ipv4组网使用::ffff:+ip地址，
ipv6组网则直接使用例如6::149，6::150）
```
./ibv_client -d hns_0 -g 3 -M push   
::ffff:10.0.0.1,::ffff:10.0.0.2,::ffff:10.0.0.3
```
Pull操作与all操作使用与上相同，将-M后参数改变即可，all操作即先跑完pull再跑push操作，不需要重复建立连接。


**FAQ**


Q：为什么1对1出现测试出带宽只有140Gbps？
A：这里原因有两个：①测出打满带宽是需要设置一定参数的，例如这里可以设置qp连接数为2，即可将带宽打满到181Gbps左右
②：将mtu报文设置为8K即以上，因为工具目前使用的是1k的报文传输，报文必结构开销占整个报文的9.5%，实际传输的数据为数据量×1.1左右,即真实测得带宽为181Gbps×1.1≈199Gbps

Q:工具编译报错 /libibverbs.so: undefined reference to ''：？
A：这个是由于缺少依赖库导致，可以看版本是否匹配，或者换一台机器编译，编译成功后直接拷贝过去或可以查看makefile文件找专家协助。

Q：如何设置报文大小？
A：目前工具暂不支持报文结构大小设置。

Q：如何使用多线程？
A：多线程需要使用cgroup打开内核才可以使用，-M参数后修改为multipush/multipull/multi既可，建议client使用multi与server使用all对应，多线程旨在在多qp/多机提高cpu瓶颈，对性能提示并不大。


