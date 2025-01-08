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

#ifndef HCCL_TEST_COMMON_H_
#define HCCL_TEST_COMMON_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <vector>
#include <memory>
#include <hccl/hccl.h>
#include <hccl/hccl_types.h>
#include <limits.h>
#include <ctype.h>
#include "acl/acl.h"
#include "acl/acl_prof.h"
#include "hccl_test_communicate.h"
#include "hccl_test_logger.h"

#undef INT_MAX
#define INT_MAX __INT_MAX__

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

struct DataSize {
    u64 minBytes;
    u64 maxBytes;
    u64 stepBytes = 0;
    double stepFactor;
    u64 count;
    u64 dataSize;
    u64 typeSize;
};

const int SERVER_MAX_DEV_NUM = 8;

#define ACLCHECK(ret) do { \
    if ((ret) != ACL_SUCCESS) { \
        ERROR("acl interface return err %s:%d, retcode: %d ", __FILE__, __LINE__, (ret)); \
        return (ret); \
    } \
} while (0)

#define HCCLCHECK(ret) do {  \
    if ((ret) != HCCL_SUCCESS) { \
        ERROR("hccl interface return errreturn err %s:%d, retcode: %d ", __FILE__, __LINE__, (ret)); \
        return (ret); \
    } \
} while (0)

#define HCCLROOTRANKCHECK(ret) do {  \
    if ((ret) != HCCL_SUCCESS && (ret) != HCCL_E_PARA) { \
        ERROR("hccl interface return errreturn err %s:%d, retcode: %d ", __FILE__, __LINE__, (ret)); \
        return (ret); \
    } \
} while (0)

namespace hccl {
class HcclTest {
public:
    HcclTest();
    virtual ~HcclTest();

    void PrintHelp();
    static struct option longopts[];

    int ParseOpt(int opt);
    int ParseCmdLine(int argc, char* argv[]);

    int CheckDataCount();
    int CheckCmdLine();

    // 计算当前进程rank号, 同一个服务器内的rank从0开始编号[0,nDev-1]
    int GetMpiProc();

    int getAviDevs(const char* devs, std::vector<int>& dev_ids);

    virtual int HcclOpBaseTestMain();

    int InitHcclComm();

    int OpbaseTestByDataSize(HcclTest* hccl_test);

    int DestoryHcclComm();

    int GetEnvResource();
    int ReleaseEnvResource();
    int InitCommunicater();

private:
    int SetDeviceSatMode();

public:
    std::shared_ptr<hccl::HcclCommunicater> communicater = nullptr;
    std::string server_ip = "";
    int server_port = -1;
    DataSize *data;
    long data_parsed_begin = 64*1024*1024;
    long data_parsed_end = 64*1024*1024;
    int64_t temp_step_bytes = 0;
    int iters = 20;
    int op_type = HCCL_REDUCE_SUM;
    int dtype = HCCL_DATA_TYPE_FP32;
    int hccl_root = 0;
    int warmup_iters = 5;
    int check = 1;
    u32 dev_count = 0;
    int npus = -1;
    int stepfactor_flag = 0;
    int stepbytes_flag = 0;
    int op_flag = 0;
    // 当前进程在通信域(MPI_COMM_WORLD)内的进程号
    int proc_rank = 0;
    // 通信域(MPI_COMM_WORLD)中的总进程数
    int proc_size = 0;
    // 当前进程在服务器内的rank号，每个服务器内的rank号都是从0开始索引
    int local_rank = 0;
    int root_rank = 0;

    int dev_id = 0;
    int rank_id = 0;
    int rank_size = 0;

    aclrtStream stream;
    HcclComm hccl_comm;
    HcclRootInfo comm_id;

    aclrtEvent start_event, end_event;

    bool print_header = true;
    bool print_dump = true;

    int profiling_flag = 0;
    aclprofConfig* profiling_config = NULL;
};
} // namespace hccl

#endif
