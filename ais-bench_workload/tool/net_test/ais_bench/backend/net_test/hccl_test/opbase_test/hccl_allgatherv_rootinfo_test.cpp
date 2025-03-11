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
#include "hccl_allgatherv_rootinfo_test.h"
#include "hccl_opbase_rootinfo_base.h"
#include "hccl_check_buf_init.h"

using namespace hccl;

HcclTest* InitOpbasePtr(HcclTest* opbase)
{
    opbase = new hccl::HcclOpBaseAllgathervTest();

    return opbase;
}

void DeleteOpbasePtr(HcclTest* opbase)
{
    delete opbase;
    opbase = nullptr;
    return;
}

namespace hccl {
HcclOpBaseAllgathervTest::HcclOpBaseAllgathervTest() : HcclOpBaseTest()
{
    host_buf = nullptr;
    recv_buff_temp = nullptr;
    check_buf = nullptr;
    send_buff = nullptr;
    recv_buff = nullptr;
}

HcclOpBaseAllgathervTest::~HcclOpBaseAllgathervTest()
{
}

int HcclOpBaseAllgathervTest::InitBufVal()
{
    // 初始化输入内存
    ACLCHECK(aclrtMallocHost((void**)&host_buf, malloc_kSize));
    HcclHostBufInit((char*)host_buf, data->count, dtype, val);
    // 初始化校验内存
    ACLCHECK(aclrtMallocHost((void**)&check_buf, malloc_kSize * rank_size));
    HcclHostBufInit((char*)check_buf, data->count * rank_size, dtype, val);

    ACLCHECK(aclrtMemcpy((void*)send_buff, malloc_kSize, (void*)host_buf, malloc_kSize, ACL_MEMCPY_HOST_TO_DEVICE));
    return 0;
}

int HcclOpBaseAllgathervTest::CheckBufResult()
{
    // 获取输出内存
    ACLCHECK(aclrtMallocHost((void**)&recv_buff_temp, malloc_kSize * rank_size));
    ACLCHECK(aclrtMemcpy((void*)recv_buff_temp, malloc_kSize * rank_size, (void*)recv_buff,
        malloc_kSize * rank_size, ACL_MEMCPY_DEVICE_TO_HOST));
    int ret = 0;
    switch (dtype) {
        case HCCL_DATA_TYPE_FP32:
            ret = CheckBufResultFloat((char*)recv_buff_temp, (char*)check_buf, data->count * rank_size);
            break;
        case HCCL_DATA_TYPE_INT8:
        case HCCL_DATA_TYPE_UINT8:
            ret = CheckBufResultInt8((char*)recv_buff_temp, (char*)check_buf, data->count * rank_size);
            break;
        case HCCL_DATA_TYPE_INT32:
        case HCCL_DATA_TYPE_UINT32:
            ret = CheckBufResultInt32((char*)recv_buff_temp, (char*)check_buf, data->count * rank_size);
            break;
        case HCCL_DATA_TYPE_FP16:
        case HCCL_DATA_TYPE_INT16:
        case HCCL_DATA_TYPE_UINT16:
        case HCCL_DATA_TYPE_BFP16:
            ret = CheckBufResultHalf((char*)recv_buff_temp, (char*)check_buf, data->count * rank_size);
            break;
        case HCCL_DATA_TYPE_INT64:
        case HCCL_DATA_TYPE_FP64:
            ret = CheckBufResultInt64((char*)recv_buff_temp, (char*)check_buf, data->count * rank_size);
            break;
        case HCCL_DATA_TYPE_UINT64:
            ret = CheckBufResultU64((char*)recv_buff_temp, (char*)check_buf, data->count * rank_size);
            break;
        default:
            ret++;
            ERROR("No match datatype.");
            break;
    }
    if (ret != 0) {
        check_err++;
    }
    return 0;
}

int HcclOpBaseAllgathervTest::CalExecutionTime(float time)
{
    double total_time_us              = time * 1000;
    double average_time_us            = total_time_us / iters;
    double algorithm_bandwith_GBytes_s = malloc_kSize * rank_size / average_time_us * B_US_TO_GB_S;

    return PrintExecutionTime(average_time_us, algorithm_bandwith_GBytes_s);
}

int HcclOpBaseAllgathervTest::DestoryCheckBuf()
{
    ACLCHECK(aclrtFreeHost(host_buf));
    ACLCHECK(aclrtFreeHost(recv_buff_temp));
    ACLCHECK(aclrtFreeHost(check_buf));
    return 0;
}

int HcclOpBaseAllgathervTest::HcclOpBaseTestMain() // 主函数
{
    if (op_flag != 0 && rank_id == root_rank) {
        WARN("The -o,--op <sum/prod/min/max> option does not take effect. Check the cmd parameter.\n");
    }

    // 获取数据量和数据类型
    InitDataCount();

    data->count = (data->count + rank_size - 1) / rank_size;
    malloc_kSize = data->count * data->typeSize;

    // 申请集合通信操作的内存
    ACLCHECK(aclrtMalloc((void**)&send_buff, malloc_kSize, ACL_MEM_MALLOC_HUGE_FIRST));
    ACLCHECK(aclrtMalloc((void**)&recv_buff, malloc_kSize * rank_size, ACL_MEM_MALLOC_HUGE_FIRST));

    // 申请recv_counts和recv_disp
    auto recv_counts = std::vector<u64>(rank_size, data->count);
    auto recv_disp = std::vector<u64>{0};
    for (size_t i = 1; i < rank_size; ++i) {
        recv_disp.emplace_back(recv_disp[i - 1] + recv_counts[i - 1]);
    }

    if (check == 1) {
        ACLCHECK(InitBufVal()); // 准备校验内存
    }

    // 执行集合通信操作
    for (int j = 0; j < warmup_iters; ++j) {
        HCCLCHECK(
            HcclAllGatherV(
                (void *)send_buff,
                data->count,
                (void*)recv_buff,
                recv_counts.data(),
                recv_disp.data(),
                (HcclDataType)dtype,
                hccl_comm,
                stream)
        );
    }

    ACLCHECK(aclrtRecordEvent(start_event, stream));

    for (int i = 0; i < iters; ++i) {
        HCCLCHECK(
            HcclAllGatherV(
                (void *)send_buff,
                data->count,
                (void*)recv_buff,
                recv_counts.data(),
                recv_disp.data(),
                (HcclDataType)dtype,
                hccl_comm,
                stream)
        );
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
    if (check == 1) {
        ACLCHECK(DestoryCheckBuf());
    }
    return ret;
}
}