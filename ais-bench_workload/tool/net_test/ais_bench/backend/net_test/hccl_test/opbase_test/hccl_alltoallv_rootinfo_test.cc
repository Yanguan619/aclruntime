/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>
#include <hccl/hccl_types.h>
#include "hccl_alltoallv_rootinfo_test.h"
#include "hccl_opbase_rootinfo_base.h"
#include "hccl_check_buf_init.h"
using namespace hccl;

HcclTest* InitOpbasePtr(HcclTest* opbase)
{
    opbase = new HcclOpBaseAlltoallvTest();

    return opbase;
}

void DeleteOpbasePtr(HcclTest* opbase)
{
    delete opbase;
    opbase = nullptr;
    return;
}

namespace hccl {
HcclOpBaseAlltoallvTest::HcclOpBaseAlltoallvTest() : HcclOpBaseTest()
{

    host_buf = nullptr;
    recv_buff_temp = nullptr;
    send_buff = nullptr;
    send_counts = nullptr;
    send_disp = nullptr;
    recv_buff = nullptr;
    recv_counts = nullptr;
    recv_disp = nullptr;
}

HcclOpBaseAlltoallvTest::~HcclOpBaseAlltoallvTest()
{
}

void HcclOpBaseAlltoallvTest::MallocSendRecvBuf()
{
    send_counts = (unsigned long long *)malloc(rank_size * sizeof(unsigned long long));
    send_disp = (unsigned long long *)malloc(rank_size * sizeof(unsigned long long));
    for (int i = 0; i < rank_size; ++i) {
        send_counts[i] = data->count / rank_size;
        send_disp[i] = i * data->count / rank_size;
    }

    recv_counts = (unsigned long long *)malloc(rank_size * sizeof(unsigned long long));
    recv_disp = (unsigned long long *)malloc(rank_size * sizeof(unsigned long long));
    for (int i = 0; i < rank_size; ++i) {
        recv_counts[i] = data->count / rank_size;
        recv_disp[i] = i * data->count / rank_size;
    }
    return;
}

int HcclOpBaseAlltoallvTest::InitBufVal()
{
    // 初始化输入内存
    ACLCHECK(aclrtMallocHost((void**)&host_buf, malloc_kSize));
    HcclHostBufInit((char*)host_buf, data->count, dtype, rank_id + 1);

    ACLCHECK(aclrtMemcpy((void*)send_buff, malloc_kSize, (void*)host_buf, malloc_kSize, ACL_MEMCPY_HOST_TO_DEVICE));
    return 0;
}

int HcclOpBaseAlltoallvTest::CheckBufResult()
{
    // 获取输出内存
    ACLCHECK(aclrtMallocHost((void**)&check_buf, malloc_kSize));
    ACLCHECK(aclrtMemcpy((void*)check_buf, malloc_kSize, (void*)recv_buff, malloc_kSize, ACL_MEMCPY_DEVICE_TO_HOST));

    int ret = 0;
    ret = HcclAlltoallvCheckResult(check_buf, recv_counts, recv_disp, rank_id, rank_size, dtype);
    if (ret != 0) {
        check_err++;
    }
    return 0;
}

int HcclOpBaseAlltoallvTest::CalExecutionTime(float time)
{
    double total_time_us = time * 1000;
    double average_time_us = total_time_us / iters;
    double algorithm_bandwith_GBytes_s = malloc_kSize / average_time_us * B_US_TO_GB_S;

    return PrintExecutionTime(average_time_us, algorithm_bandwith_GBytes_s);
}

void HcclOpBaseAlltoallvTest::FreeSendRecvBuf()
{
    free(send_counts);
    free(send_disp);
    free(recv_counts);
    free(recv_disp);
}

int HcclOpBaseAlltoallvTest::DestoryCheckBuf()
{
    ACLCHECK(aclrtFreeHost(host_buf));
    ACLCHECK(aclrtFreeHost(check_buf));
    return 0;
}

int HcclOpBaseAlltoallvTest::HcclOpBaseTestMain() // 主函数
{
    if (op_flag != 0 && rank_id == root_rank) {
        WARN("The -o,--op <sum/prod/min/max> option does not take effect. Check the cmd parameter.\n");
    }
    // 获取数据量和数据类型
    InitDataCount();
    data->count = (data->count + rank_size - 1) / rank_size * rank_size;
    malloc_kSize = data->count * data->typeSize;

    // 申请集合通信操作的内存
    ACLCHECK(aclrtMalloc((void**)&send_buff, malloc_kSize, ACL_MEM_MALLOC_HUGE_FIRST));
    ACLCHECK(aclrtMalloc((void**)&recv_buff, malloc_kSize, ACL_MEM_MALLOC_HUGE_FIRST));

    // 申请sendcounts和send_disp
    MallocSendRecvBuf();

    if (check == 1) {
        ACLCHECK(InitBufVal()); // 准备校验内存
    }

    // 执行集合通信操作
    for (int j = 0; j < warmup_iters; ++j) {
        HCCLCHECK(HcclAlltoAllV((void *)send_buff, send_counts, send_disp, (HcclDataType)dtype,\
            (void*)recv_buff, recv_counts, recv_disp, (HcclDataType)dtype, hccl_comm, stream));
    }

    ACLCHECK(aclrtRecordEvent(start_event, stream));

    for (int i = 0; i < iters; ++i) {
        HCCLCHECK(HcclAlltoAllV((void *)send_buff, send_counts, send_disp, (HcclDataType)dtype,\
            (void*)recv_buff, recv_counts, recv_disp, (HcclDataType)dtype, hccl_comm, stream));
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
    ACLCHECK(aclrtFree(send_buff));
    ACLCHECK(aclrtFree(recv_buff));
    FreeSendRecvBuf();
    if (check == 1) {
        ACLCHECK(DestoryCheckBuf());
    }
    return ret;
}
}