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

#ifndef HCCL_TEST_COMMUNICATE_H_
#define HCCL_TEST_COMMUNICATE_H_

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <fstream>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "hccl/hccl.h"
#include <hccl/hccl_types.h>

#include "hccl_test_logger.h"


namespace hccl
{
class HcclCommunicater
{
public:
    HcclCommunicater(
        const std::string serverIP,
        const int serverPort,
        const int rankSize,
        const int rankID
    );

    virtual ~HcclCommunicater();

    int SynchronizeRootInfo(
        void *dataBuffer,
        const size_t bufferSize
    );

    int AllGatherInfoToRoot(
        void *dataList,
        void *dataBuffer,
        const size_t bufferSize,
        const size_t listLen
    );

private:
    int ServerBcast(
        void *dataBuffer,
        const size_t bufferSize
    );

    int ClientRecv(
        void *dataBuffer,
        const size_t bufferSize
    );

    int ServerGather(
        void *dataList,
        void *dataBuffer,
        const size_t bufferSize,
        const size_t listLen
    );

    int ClientBcast(
        void *dataBuffer,
        const size_t bufferSize
    );

    bool ServerPreset();

    bool ClientPreset();

private:
    const std::string m_serverIP;
    const int m_serverPort;
    const int m_rankSize;
    const int m_rankID;
    int m_rootRank = 0;
    int m_serverSkt = -1;
    int m_clientSkt = -1;
};

}
#endif