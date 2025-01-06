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

#ifndef HCCL_OPBASE_ROOTINFO_BASE_H_
#define HCCL_OPBASE_ROOTINFO_BASE_H_
#include "hccl_test_common.h"
#ifdef MPI_SUPPORT
#include "mpi.h"
#endif

#include <hccl/hccl_types.h>
#include "hccl_check_common.h"
#include "hccl_check_buf_init.h"
#include "hccl_test_logger.h"

namespace hccl {
const double B_US_TO_GB_S = 1.0E6 / 1.0E9;

class HcclOpBaseTest: public HcclTest {
public:
    HcclOpBaseTest();
    virtual ~HcclOpBaseTest();

    virtual int hccl_op_base_test(); // 主函数
    virtual void init_data_count(); // 初始化malloc_kSize
    virtual void no_verification();
    virtual void is_data_overflow();
    virtual int print_execution_time(double average_time_us, double algorithm_bandwith_GBytes_s); // 打印耗时

public:
    void *send_buff;
    void *recv_buff;
    u64 malloc_kSize = 0;
    void *host_buf;
    void *recv_buff_temp;
    void *check_buf;
    int check_err = 0;
    int val = 2; //校验参数

private:
    virtual int init_buf_val();  //（初始化host_buf，初始化check_buf，拷贝到send_buf） 其中需要调用HcclHostBufInit
    virtual int check_buf_result(); //（recv_buf拷贝到recvbufftemp,并且校验正确性）需要调用check_buf_init，校验正确性要调用CheckBufResultFloat

    virtual int destory_check_buf(); // 校验内存销毁

    const char *data_size = {"data_size(Bytes):"};
    const char *aveg_time = {"aveg_time(us):"};
    const char *alg_bandwidth = {"alg_bandwidth(GB/s):"};
    const char *verification_result = {"check_result:"};
};
}
#endif
