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

#include <algorithm> // For std::copy_n
#include "hccl_test_communicate.h"

namespace hccl {
const int RETRY_COUNT = 10;
const int RETRY_INTERVAL = 1; // second
const int RETRY_TIMES = 5;

int SafeCopy(char *SrcAddrStart, char *SrcAddrEnd, char *TargetAddrStart)
{
    try {
        std::copy(SrcAddrStart, SrcAddrEnd, TargetAddrStart);
    } catch (const std::bad_alloc& e) {
        ERROR("copy Error occurred. %s", e.what());
        return 1;
    } catch (const std::length_error& e) {
        ERROR("Error: Input sequence has zero length. %s", e.what());
        return 1;
    } catch (const std::exception& e) {
        ERROR("Unexpected error occurred: %s", e.what());
        return 1;
    }
    return 0;
}

HcclCommunicater::HcclCommunicater(
    const std::string serverIP,
    const int serverPort,
    const int rankSize,
    const int rankID
): m_serverIP(serverIP), m_serverPort(serverPort), m_rankSize(rankSize), m_rankID(rankID)
{
}

HcclCommunicater::~HcclCommunicater()
{
    close(m_clientSkt);
    close(m_serverSkt);
}

int HcclCommunicater::SynchronizeRootInfo(
    void *dataBuffer,
    const size_t bufferSize
)
{
    if (m_rankID == m_rootRank) {
        return ServerBcast(dataBuffer, bufferSize);
    } else {
        return ClientRecv(dataBuffer, bufferSize);
    }
}

int HcclCommunicater::AllGatherInfoToRoot(
    void *dataList,
    void *dataBuffer,
    const size_t bufferSize,
    const size_t listLen
)
{
    if (m_rankID == m_rootRank) {
        return ServerGather(dataList, dataBuffer, bufferSize, listLen);
    } else {
        return ClientBcast(dataBuffer, bufferSize); // databuffer 暂时未知
    }
}

int HcclCommunicater::ServerBcast(
    void *dataBuffer,
    const size_t bufferSize
)
{
    ServerPreset();
    DEBUG("Rank: %d, Server Bcast listening on port: %d ......", m_rankID, m_serverPort);
    int connectedClientCount = 0;
    int tryConnectCount = 0;
    int clientRank = -1;
    while (connectedClientCount < m_rankSize - 1) {
        tryConnectCount++;
        if (tryConnectCount >= m_rankSize * RETRY_TIMES) {
            close(m_serverSkt);
            ERROR("Root rank: %d, Server broadcast stopped after try %d times.", m_rankID, tryConnectCount);
            return -1;
        }
        struct sockaddr_in clientAddr;
        socklen_t clientAddrlen = sizeof(clientAddr);
        int clientSkt = accept(m_serverSkt, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrlen);
        if (clientSkt == -1) {
            DEBUG("rank: %d, accepting client connection failed! Retry after %d sec", m_rankID, RETRY_INTERVAL);
            sleep(RETRY_INTERVAL);
            continue;
        }
        DEBUG("rank: %d, Client connected from %s", m_rankID, inet_ntoa(clientAddr.sin_addr));
        for (int i = 0; i < RETRY_COUNT; i++) {
            if (send(clientSkt, static_cast<char*>(dataBuffer), bufferSize, 0) <= 0) {continue;}
            DEBUG("server rank: %d, send rootInfo to client success!", m_rankID);
            if (recv(clientSkt, &clientRank, sizeof(int), 0) <= 0) {continue;}
            DEBUG("server rank: %d recv rank: %d from client success!", m_rankID, clientRank);
            break;
        }
        if (clientRank >= m_rankSize) {
            WARN("ClientRank: %d is over max rankID: %d, won't recv!", m_rankID, m_rankSize - 1);
            close(clientSkt);
            continue;
        }
        ++connectedClientCount;
        close(clientSkt);
    }
    close(m_serverSkt);
    DEBUG("root rank: %d, Server broadcast stopped after serving %d clients.", m_rankID, m_rankSize - 1);
    return 0;
}

int HcclCommunicater::ClientRecv(
    void *dataBuffer,
    const size_t bufferSize
)
{
    ClientPreset();
    struct sockaddr_in serverAddr;
    // 设置服务器地址
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(m_serverIP.c_str());
    serverAddr.sin_port = htons(m_serverPort);
    for (int i = 0; i < RETRY_COUNT; i++) {
        while (true) {
            if (connect(m_clientSkt, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
                sleep(RETRY_INTERVAL);
                continue;
            }
            break;
        }
        DEBUG("rank: %d, Client Recv connect server success! ", m_rankID);
        for (int j = 0; j < RETRY_COUNT; j++) {
            if (recv(m_clientSkt, static_cast<char*>(dataBuffer), bufferSize, 0) <= 0) {continue;}
            DEBUG("rank: %d, recv from rootInfo from server success! ", m_rankID);
            if (send(m_clientSkt, &m_rankID, sizeof(int), 0) <= 0) {continue;}
            DEBUG("rank: %d, reply rank to server success! ", m_rankID);
            close(m_clientSkt);
            DEBUG("rank: %d, client received from server success!", m_rankID);
            return 0;
        }
    }
    ERROR("Rank: %d, client received from server failed!", m_rankID);
    return -1;
}

int HcclCommunicater::ServerGather(
    void *dataList,
    void *dataBuffer,
    const size_t bufferSize,
    const size_t listLen
)
{
    ServerPreset();
    DEBUG("rank: %d, Server Gather start listening on port: %d", m_rankID, m_serverPort);
    int connectedClientCount = 0;
    int tryConnectCount = 0;
    int clientRank = -1;
    char* singleData = nullptr;
    singleData = static_cast<char*>(malloc(bufferSize * sizeof(char)));
    int ret = SafeCopy(static_cast<char*>(dataBuffer), static_cast<char*>(dataBuffer) + bufferSize,
        static_cast<char*>(dataList)); // copy root rank data

    if (ret != 0) {
        free(singleData);
        return ret;
    }

    while (connectedClientCount < m_rankSize - 1) {
        tryConnectCount++;
        if (tryConnectCount >= m_rankSize * RETRY_TIMES) {
            free(singleData);
            close(m_serverSkt);
            ERROR("Root rank: %d, Server broadcast stopped after try %d times.", m_rankID, tryConnectCount);
            return -1;
        }
        struct sockaddr_in clientAddr;
        socklen_t clientAddrlen = sizeof(clientAddr);
        int clientSkt = accept(m_serverSkt, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrlen);
        if (clientSkt == -1) {
            DEBUG("rank: %d, accepting client connection failed!", m_rankID);
            continue;
        }
        DEBUG("rank: %d, Client connected from %s, accepted clientSkt: %d",
            m_rankID, inet_ntoa(clientAddr.sin_addr), clientSkt);
        if (recv(clientSkt, &clientRank, sizeof(int), 0) <= 0) {continue;}
        if (clientRank >= listLen) {
            DEBUG("clientRank: %d is over max rankID: %zu, won't recv!", m_rankID, listLen - 1);
            continue;
        }
        DEBUG("server recv client rank: %d success!", m_rankID);
        for (int i = 0; i < RETRY_COUNT; i++) {
            if (recv(clientSkt, singleData, bufferSize, 0) <= 0) {continue;}
            DEBUG("server recv data from rank: %d success!", m_rankID);
            break;
        }
        if (send(clientSkt, &clientRank, sizeof(int), 0) <= 0) {continue;}
        DEBUG("server reply rank %d to client success!", clientRank);
        ret = SafeCopy(singleData, singleData + bufferSize, static_cast<char*>(dataList));

        if (ret != 0) {
            free(singleData);
            return ret;
        }

        ++connectedClientCount;
        close(clientSkt);
    }
    free(singleData);
    close(m_serverSkt);
    DEBUG("rank: %d, Server gather stopped after serving %d clients.", m_rankID, m_rankSize - 1);
    return 0;
}

int HcclCommunicater::ClientBcast(
    void *dataBuffer,
    const size_t bufferSize
)
{
    ClientPreset();
    DEBUG("rank: %d,  start ClientBcast! ", m_rankID);
    struct sockaddr_in serverAddr;
    // 设置服务器地址
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(m_serverIP.c_str());
    serverAddr.sin_port = htons(m_serverPort);
    int connectedClientCount = -1;
    int retInfo = -1;
    for (int i = 0; i < RETRY_COUNT; i++) {
        while (true) {
            if (connect(m_clientSkt, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
                sleep(RETRY_INTERVAL);
                continue;
            }
            DEBUG("rank: %d, Client Bcast connect server success! ", m_rankID);
            break;
        }
        if (send(m_clientSkt, &m_rankID, sizeof(int), 0) <= 0) {continue;}
        DEBUG("rank: %d, client send rank info success! ", m_rankID);
        if (send(m_clientSkt, static_cast<char*>(dataBuffer), bufferSize, 0) <= 0) {continue;}
        DEBUG("rank: %d, client send data success! ", m_rankID);
        for (int i = 0; i < RETRY_COUNT; i++) {
            if (recv(m_clientSkt, &retInfo, sizeof(int), 0) <= 0) {continue;}
            DEBUG("rank: %d, recv retInfo %d success! ", m_rankID, retInfo);
            if (retInfo != m_rankID) {break;}
            close(m_clientSkt);
            DEBUG("client rank: %d, broadcast success!", m_rankID);
            return 0;
        }
    }
    close(m_clientSkt);
    DEBUG("Client rank: %d, broadcast failed!", m_rankID);
    return -1;
}

bool HcclCommunicater::ServerPreset()
{
    // 服务器套接字校验
    m_serverSkt = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverSkt == -1) {
        ERROR("Rank: %d, create socket failed.", m_rankID);
        return false;
    }
    DEBUG("rank: %d, serverSkt: %d", m_rankID, m_serverSkt);

    int reuse = 1;
    if (setsockopt(m_serverSkt, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        ERROR("Rank: %d, setsocket options failed.", m_rankID);
        close(m_serverSkt);
        return false;
    }

    struct sockaddr_in serverAddr;
    // 设置服务器地址
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(m_serverIP.c_str());
    serverAddr.sin_port = htons(m_serverPort);

    // 绑定服务器套接字
    for (int i = 0; i < RETRY_COUNT; i++) {
        if (bind(m_serverSkt, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            DEBUG("rank: %d, warning, Server bind failed, retry count: %d", m_rankID, i + 1);
            sleep(RETRY_INTERVAL);
        } else {
            break;
        }
    }

    for (int i = 0; i < RETRY_COUNT; i++) {
        if (listen(m_serverSkt, m_rankSize) == -1) {
            sleep(RETRY_INTERVAL);
        } else {
            break;
        }
    }
    return true;
}

bool HcclCommunicater::ClientPreset()
{
    // 服务器套接字校验
    m_clientSkt = socket(AF_INET, SOCK_STREAM, 0);
    if (m_clientSkt == -1) {
        ERROR("Rank: %d, create socket failed.", m_rankID);
        return false;
    }
    DEBUG("Rank: %d, , clientSkt: %d", m_rankID, m_clientSkt);
    int reuse = 1;
    if (setsockopt(m_clientSkt, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        ERROR("Rank: %d, set socket options failed.", m_rankID);
        close(m_clientSkt);
        return false;
    }
    return true;
}

}