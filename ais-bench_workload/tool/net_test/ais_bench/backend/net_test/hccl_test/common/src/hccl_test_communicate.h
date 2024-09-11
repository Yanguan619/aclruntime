#ifndef __HCCL_TEST_COMMUNICATE_H_
#define __HCCL_TEST_COMMUNICATE_H_

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

    void SetRootRank(const int rootRank);

    void SynchronizeRootInfo(
        void *dataBuffer,
        const size_t dataLen
    );

    void AllGatherInfoToRoot(
        void *dataList,
        void *dataBuffer,
        const size_t dataLen,
        const size_t listLen
    );

private:
    void ServerBcast(
        void *dataBuffer,
        const size_t dataLen
    );

    void ClientRecv(
        void *dataBuffer,
        const size_t dataLen
    );

    void ServerGather(
        void *dataList,
        void *dataBuffer,
        const size_t dataLen,
        const size_t listLen
    );

    void ClientBcast(
        void *dataBuffer,
        const size_t dataLen
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