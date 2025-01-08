#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef MPI_SUPPORT
#include "mpi.h"
#endif
#include "hccl_test_common.h"

using namespace hccl;

extern HcclTest* init_opbase_ptr(HcclTest* opbase);
extern void delete_opbase_ptr(HcclTest* opbase);

int main(int argc, char *argv[])
{
    #ifdef MPI_SUPPORT
    MPI_Init(&argc, &argv);
    #endif
    setvbuf(stdout, NULL, _IOLBF, 0); // 设置printf的缓冲区大小

    int ret = 0;

    // 构造执行器
    HcclTest *hccl_test = nullptr;
    hccl_test = init_opbase_ptr(hccl_test);
    if(hccl_test == nullptr) {
        ERROR("Init hccl executor failed.");
        ret = -1;
        goto hccltesterr3;
    }

    // 解析命令行入参
    ret = hccl_test->parse_cmd_line(argc, argv);
    if (ret == 1) {
        // 启动--help
        ret = 0;
        goto hccltesterr2;
    } else if(ret == -1) {
        // 入参解析失败
        ERROR("This is an error in parse cmd line.");
        goto hccltesterr2;
    }

    #ifndef MPI_SUPPORT
    // 初始化HcclCommunicater
    hccl_test->InitCommunicater();
    #endif

    // 查找本host上的所有MPI拉起的进程
    ret = hccl_test->get_mpi_proc();
    if (ret != 0) {
        ERROR("This is an error in get mpi proc.");
        goto hccltesterr2;
    }

    // 校验命令行参数
    ret = hccl_test->check_cmd_line();
    if (ret != 0) {
        ERROR("This is an error in check cmd line.");
        goto hccltesterr2;
    }

    // 获取hccltest的环境变量
    ret = hccl_test->get_env_resource();
    if (ret != 0) {
        ERROR("This is an error in get env resource.");
        goto hccltesterr1;
    }

    // 初始化集合通信域
    ret = hccl_test->init_hcclComm();
    if (ret != 0) {
        ERROR("This is an error in init hcclComm info.");
        goto hccltesterr2;
    }

    // 启动测试
    ret = hccl_test->opbase_test_by_data_size(hccl_test);
    if (ret != 0) {
        ERROR("This is an error in launch op base test by data size.");
        goto hccltesterr0;
    }

hccltesterr0:
    // 销毁集合通信域
    ret = hccl_test->destory_hcclComm();
    if (ret != 0) {
        ERROR("This is an error in destory hcclComm.");
    }
hccltesterr1:
    // 销毁环境变量申请的资源
    ret = hccl_test->release_env_resource();
    if (ret != 0) {
        ERROR("This is an error in release env resource.");
    }
hccltesterr2:
    // 删除构造器
    delete_opbase_ptr(hccl_test);
hccltesterr3:
    #ifdef MPI_SUPPORT
    // 释放MPI所用资源
    MPI_Finalize();
    #endif
    return ret;
}
