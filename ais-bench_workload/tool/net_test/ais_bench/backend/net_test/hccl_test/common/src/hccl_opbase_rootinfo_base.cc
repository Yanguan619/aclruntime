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
#include <string>
#include <cmath>
#include <cstdint>
#include <hccl/hccl_types.h>
#include "hccl_opbase_rootinfo_base.h"

namespace hccl {
HcclOpBaseTest::HcclOpBaseTest()
{
    host_buf = nullptr;
    recv_buff_temp = nullptr;
    check_buf = nullptr;
    send_buff = nullptr;
    recv_buff = nullptr;
}

HcclOpBaseTest::~HcclOpBaseTest()
{

}

int HcclOpBaseTest::HcclOpBaseTestMain()
{
    return 0;
}

void HcclOpBaseTest::InitDataCount()
{
    switch (dtype) {
        case HCCL_DATA_TYPE_FP32:
            data->count = (data->dataSize + sizeof(float) - 1) / sizeof(float); // count向上取整
            data->typeSize = sizeof(float);
            break;
        case HCCL_DATA_TYPE_INT32:
            data->count = (data->dataSize + sizeof(int) - 1)/sizeof(int);
            data->typeSize = sizeof(int);
            break;
        case HCCL_DATA_TYPE_BFP16:
        case HCCL_DATA_TYPE_FP16:
        case HCCL_DATA_TYPE_INT16:
            data->count = (data->dataSize + sizeof(short) - 1)/sizeof(short);
            data->typeSize = sizeof(short);
            break;
        case HCCL_DATA_TYPE_INT8:
            data->count = (data->dataSize + sizeof(signed char) - 1)/sizeof(signed char);
            data->typeSize = sizeof(signed char);
            break;
        case HCCL_DATA_TYPE_INT64:
        case HCCL_DATA_TYPE_FP64:
            data->count = (data->dataSize + sizeof(long long) - 1)/sizeof(long long);
            data->typeSize = sizeof(long long);
            break;
        case HCCL_DATA_TYPE_UINT64:
            data->count = (data->dataSize + sizeof(unsigned long long) - 1)/sizeof(unsigned long long);
            data->typeSize = sizeof(unsigned long long);
            break;
        case HCCL_DATA_TYPE_UINT8:
            data->count = (data->dataSize + sizeof(unsigned char) - 1)/sizeof(unsigned char);
            data->typeSize = sizeof(unsigned char);
            break;
        case HCCL_DATA_TYPE_UINT16:
            data->count = (data->dataSize + sizeof(unsigned short) - 1)/sizeof(unsigned short);
            data->typeSize = sizeof(unsigned short);
            break;
        case HCCL_DATA_TYPE_UINT32:
            data->count = (data->dataSize + sizeof(unsigned int) - 1)/sizeof(unsigned int);
            data->typeSize = sizeof(unsigned int);
            break;
        default:
            data->count = (data->dataSize + sizeof(float) - 1)/sizeof(float);
            data->typeSize = sizeof(float);
            break;
    }
    return;
}

int HcclOpBaseTest::InitBufVal()
{
    return 0;
}

int HcclOpBaseTest::CheckBufResult()
{
    return 0;
}

void HcclOpBaseTest::NoVerification()
{
    check = 0; // 不进行校验
    if (rank_id == root_rank && print_dump) {
        WARN("The calculation result overflows, no verification is performed.");
        print_dump = false;
    }
    return;
}

void HcclOpBaseTest::IsDataOverflow()
{
    if (op_type == HCCL_REDUCE_PROD) {
        if (dtype == HCCL_DATA_TYPE_FP16 && rank_size >= 16) {
            NoVerification();
        }
        if (dtype == HCCL_DATA_TYPE_FP32 && rank_size >= 128) {
            NoVerification();
        }
        if (dtype == HCCL_DATA_TYPE_INT8 && rank_size >= 7) {
            NoVerification();
        }
        if (dtype == HCCL_DATA_TYPE_INT32 && rank_size >= 31) {
            NoVerification();
        }
    } else if (op_type == HCCL_REDUCE_SUM) {
        if (dtype == HCCL_DATA_TYPE_INT8 && rank_size >= 63) {
            NoVerification();
        }
    }

    return;
}

int HcclOpBaseTest::PrintExecutionTime(double average_time_us, double algorithm_bandwith_GBytes_s)
{
    setvbuf(stdout, NULL, _IOLBF, 0); // 设置printf的缓冲区大小
    // 不开启结果校验场景
    if (check == 0) {
        if (rank_id == root_rank) {
            if (print_header) {
                INFO("Test result without check is:");
                LOG_ORIGIN(" %-15s | %-12s | %-18s | %s", data_size, aveg_time, alg_bandwidth, verification_result);
                print_header = false;
            }
            LOG_ORIGIN(" %-17llu | %-14.2f | %-20.5f | NULL", data->dataSize,
                average_time_us, algorithm_bandwith_GBytes_s);
        }
        return 0;
    }

    // 开启结果校验，部分rank结果校验失败场景
    bool check_result[rank_size];
    if (check_err != 0) {
        check_result[rank_id] = false; // 结果校验失败
        ERROR("Rank id %d, check result failed.", rank_id);
    } else {
        check_result[rank_id] = true; // 结果校验成功
    }

#ifdef MPI_SUPPORT
    MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, check_result, sizeof(bool), MPI_BYTE, MPI_COMM_WORLD);
#endif

#ifndef MPI_SUPPORT
    bool curResuult = check_result[rank_id];
    int ret = communicater->AllGatherInfoToRoot(&check_result, &curResuult, sizeof(bool), rank_size);
    if (ret != 0) {
        ERROR("Rank: %d run all gather root info failed! Print execution time failed!", rank_id);
        return ret;
    }
#endif


    if (rank_id == root_rank)
    {
        bool result = true;
        for (int p = 0; p < rank_size; p++) {
            if (check_result[p] == false) {
                result = false;
                break;
            }
        }
        if (print_header) {
            INFO("Test result with check is:");
            LOG_ORIGIN(" %-15s | %-12s | %-18s | %s", data_size, aveg_time, alg_bandwidth, verification_result);
            print_header = false;
        }

        if (!result) {
            LOG_ORIGIN(" %-17llu | %-14.2f | %-20.5f | failed", data->dataSize,
                average_time_us, algorithm_bandwith_GBytes_s);
        } else {
            LOG_ORIGIN(" %-17llu | %-14.2f | %-20.5f | success", data->dataSize,
                average_time_us, algorithm_bandwith_GBytes_s);
        }
    }
    return 0;
}


int HcclOpBaseTest::DestoryCheckBuf()
{
    return 0;
}
}