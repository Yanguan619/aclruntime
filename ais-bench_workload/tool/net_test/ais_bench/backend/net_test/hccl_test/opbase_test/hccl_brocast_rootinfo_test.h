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

#ifndef __HCCL_BROCAST_ROOTINFO_TEST_H_
#define __HCCL_BROCAST_ROOTINFO_TEST_H_
#include "hccl_test_common.h"
#ifdef MPI_SUPPORT
#include "mpi.h"
#endif
#include "hccl_check_common.h"
#include "hccl_opbase_rootinfo_base.h"
#include "hccl_test_logger.h"

namespace hccl {
class HcclOpBaseBrocastTest:public HcclOpBaseTest
{
public:
    HcclOpBaseBrocastTest();
    virtual ~HcclOpBaseBrocastTest();

    virtual int hccl_op_base_test(); //主函数
private:
    virtual int init_buf_val();  //（初始化host_buf，初始化check_buf，拷贝到send_buf） 其中需要调用hccl_host_buf_init
    virtual int check_buf_result();//（recv_buf拷贝到recvbufftemp,并且校验正确性）需要调用check_buf_init，校验正确性要调用check_buf_result_float
    int cal_execution_time(float time);//统计耗时
    virtual int destory_check_buf();//集合通信销毁
    void *buff;
};
}
#endif