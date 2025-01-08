#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>
#include <hccl/hccl_types.h>
#include "hccl_brocast_rootinfo_test.h"
#include "hccl_opbase_rootinfo_base.h"
#include "hccl_check_buf_init.h"
using namespace hccl;

HcclTest* InitOpbasePtr(HcclTest* opbase)
{
    opbase = new HcclOpBaseBrocastTest();

    return opbase;
}

void DeleteOpbasePtr(HcclTest* opbase)
{
    delete opbase;
    opbase = nullptr;
    return;
}

namespace hccl
{
HcclOpBaseBrocastTest::HcclOpBaseBrocastTest() : HcclOpBaseTest()
{

    host_buf = nullptr;
    recv_buff_temp = nullptr;
    check_buf = nullptr;
    buff = nullptr;
}

HcclOpBaseBrocastTest::~HcclOpBaseBrocastTest()
{

}

int HcclOpBaseBrocastTest::InitBufVal()
{
    // 初始化输入内存
    ACLCHECK(aclrtMallocHost((void**)&host_buf, malloc_kSize));
    if(rank_id == root_rank)
    {
        HcclHostBufInit((char*)host_buf, data->count, dtype, val);
        ACLCHECK(aclrtMemcpy((void*)buff, malloc_kSize, (void*)host_buf, malloc_kSize, ACL_MEMCPY_HOST_TO_DEVICE));
    }
    // 初始化校验内存
    ACLCHECK(aclrtMallocHost((void**)&check_buf, malloc_kSize));
    HcclHostBufInit((char*)check_buf, data->count, dtype, val);

    return 0;
}

int HcclOpBaseBrocastTest::CheckBufResult()
{
    // 获取输出内存
    ACLCHECK(aclrtMallocHost((void**)&recv_buff_temp, malloc_kSize));
    ACLCHECK(aclrtMemcpy((void*)recv_buff_temp, malloc_kSize, (void*)buff, malloc_kSize, ACL_MEMCPY_DEVICE_TO_HOST));
    int ret = 0;
    switch(dtype)
    {
        case HCCL_DATA_TYPE_FP32:
            ret = CheckBufResultFloat((char*)recv_buff_temp, (char*)check_buf, data->count);
            break;
        case HCCL_DATA_TYPE_INT8:
        case HCCL_DATA_TYPE_UINT8:
            ret = CheckBufResultInt8((char*)recv_buff_temp, (char*)check_buf, data->count);
            break;
        case HCCL_DATA_TYPE_INT32:
        case HCCL_DATA_TYPE_UINT32:
            ret = CheckBufResultInt32((char*)recv_buff_temp, (char*)check_buf, data->count);
            break;
        case HCCL_DATA_TYPE_FP16:
        case HCCL_DATA_TYPE_INT16:
        case HCCL_DATA_TYPE_UINT16:
        case HCCL_DATA_TYPE_BFP16:
            ret = CheckBufResultHalf((char*)recv_buff_temp, (char*)check_buf, data->count);
            break;
        case HCCL_DATA_TYPE_INT64:
        case HCCL_DATA_TYPE_FP64:
            ret = CheckBufResultInt64((char*)recv_buff_temp, (char*)check_buf, data->count);
            break;
        case HCCL_DATA_TYPE_UINT64:
            ret = CheckBufResultU64((char*)recv_buff_temp, (char*)check_buf, data->count);
            break;
        default:
            ret++;
            ERROR("No match datatype.");
            break;
    }
    if(ret != 0)
    {
        check_err++;
    }
    return 0;
}

int HcclOpBaseBrocastTest::CalExecutionTime(float time)
{
    double total_time_us = time * 1000;
    double average_time_us = total_time_us / iters;
    double algorithm_bandwith_GBytes_s = malloc_kSize / average_time_us * B_US_TO_GB_S;

    return PrintExecutionTime(average_time_us, algorithm_bandwith_GBytes_s);
}

int HcclOpBaseBrocastTest::DestoryCheckBuf()
{
    ACLCHECK(aclrtFreeHost(host_buf));
    ACLCHECK(aclrtFreeHost(recv_buff_temp));
    ACLCHECK(aclrtFreeHost(check_buf));
    return 0;
}

int HcclOpBaseBrocastTest::HcclOpBaseTestMain() // 主函数
{
    if (op_flag != 0 && rank_id == root_rank) {
        WARN("The -o,--op <sum/prod/min/max> option does not take effect. Check the cmd parameter.\n");
    }
    // 获取数据量和数据类型
    InitDataCount();

    malloc_kSize = data->count * data->typeSize;

    // 申请集合通信操作的内存
    ACLCHECK(aclrtMalloc((void**)&buff, malloc_kSize, ACL_MEM_MALLOC_HUGE_FIRST));

    if (check == 1) {
        ACLCHECK(InitBufVal()); // 准备校验内存
    }

    // 执行集合通信操作
    for(int j = 0; j < warmup_iters; ++j) {
        HCCLCHECK(HcclBroadcast((void *)buff, data->count, (HcclDataType)dtype, root_rank, hccl_comm, stream));
    }

    ACLCHECK(aclrtRecordEvent(start_event, stream));

    for(int i = 0; i < iters; ++i) {
        HCCLCHECK(HcclBroadcast((void *)buff, data->count, (HcclDataType)dtype, root_rank, hccl_comm, stream));
    }
    // 等待stream中集合通信任务执行完成
    ACLCHECK(aclrtRecordEvent(end_event, stream));

    ACLCHECK(aclrtSynchronizeStream(stream));

    float time;
    ACLCHECK(aclrtEventElapsedTime(&time, start_event, end_event));

    if (check == 1) {
        ACLCHECK(CheckBufResult()); // 校验计算结果
    }

    int ret = CalExecutionTime(time);

    // 销毁集合通信内存资源
    ACLCHECK(aclrtFree(buff));
    if (check == 1) {
        ACLCHECK(DestoryCheckBuf());
    }
    return ret;
}
}