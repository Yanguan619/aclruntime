#include "hccl_test_communicate.h"

namespace hccl
{
const int RETRY_COUNT = 10;
const int RETRY_INTERVAL = 1; // second

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

void HcclCommunicater::SetRootRank(const int rootRank)
{
    m_rootRank = rootRank;
}

void HcclCommunicater::SynchronizeRootInfo(
    void *dataBuffer,
    const size_t dataLen
)
{
    if (m_rankID == m_rootRank) {
        ServerBcast(dataBuffer, dataLen);
    } else {
        ClientRecv(dataBuffer, dataLen);
    }
}

void HcclCommunicater::AllGatherInfoToRoot(
    void *dataList,
    void *dataBuffer,
    const size_t dataLen,
    const size_t listLen
)
{
    if (m_rankID == m_rootRank) {
        ServerGather(dataList, dataBuffer, dataLen, listLen);
    } else {
        ClientBcast(dataBuffer, dataLen); // databuffer 暂时未知
    }
}

void HcclCommunicater::ServerBcast(
    void *dataBuffer,
    const size_t dataLen
)
{
    ServerPreset();
    DEBUG("rank: %d, Server Bcast listening on port: %d ......", m_rankID, m_serverPort);
    int connectedClientCount = 0;
    while (connectedClientCount < m_rankSize - 1) {
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
            if (send(clientSkt, static_cast<char*>(dataBuffer), dataLen, 0) <= 0) {
                sleep(RETRY_INTERVAL);
            } else {
                break;
            }
        }
        ++connectedClientCount;
        close(clientSkt);
    }
    close(m_serverSkt);
    DEBUG("rank: %d, Server broadcast stopped after serving %d clients.", m_rankID, m_rankSize - 1);
}

void HcclCommunicater::ClientRecv(
    void *dataBuffer,
    const size_t dataLen
)
{
    ClientPreset();
    struct sockaddr_in serverAddr;
    // 设置服务器地址
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(m_serverIP.c_str());
    serverAddr.sin_port = htons(m_serverPort);
    while (true) {
        if (connect(m_clientSkt, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
            sleep(RETRY_INTERVAL);
            continue;
        }
        DEBUG("rank: %d, connect server success! ", m_rankID);
        if (recv(m_clientSkt, static_cast<char*>(dataBuffer), dataLen, 0) <= 0) {
            sleep(RETRY_INTERVAL);
            continue;
        }
        break;
    }
    close(m_clientSkt);
    DEBUG("rank: %d, client received from server success!", m_rankID);
}

void HcclCommunicater::ServerGather(
    void *dataList,
    void *dataBuffer,
    const size_t dataLen,
    const size_t listLen
)
{
    ServerPreset();
    DEBUG("rank: %d, Server Gather start listening on port: %d", m_rankID, m_serverPort);
    int connectedClientCount = 0;
    int clientRank = -1;
    char* singleData = nullptr;
    singleData = static_cast<char*>(malloc(dataLen * sizeof(char)));
    memcpy(static_cast<char*>(dataList), static_cast<char*>(dataBuffer), dataLen); // copy root rank data

    while (connectedClientCount < m_rankSize - 1) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrlen = sizeof(clientAddr);
        int clientSkt = accept(m_serverSkt, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrlen);
        if (clientSkt == -1) {
            WARN("rank: %d, accepting client connection failed!", m_rankID);
            continue;
        }
        DEBUG("rank: %d, Client connected from %s, accepted clientSkt: %d",
            m_rankID, inet_ntoa(clientAddr.sin_addr), clientSkt);
        while (send(clientSkt, &connectedClientCount, sizeof(int), 0) <= 0) {sleep(RETRY_INTERVAL);}
        while (recv(clientSkt, &clientRank, sizeof(int), 0) <= 0) {sleep(RETRY_INTERVAL);}
        while (recv(clientSkt, singleData, dataLen, 0) <= 0) {sleep(RETRY_INTERVAL);}
        if (clientRank >= listLen) {
            WARN("clientRank: %d is over max rankID: %zu, won't recv!", m_rankID, listLen - 1);
            continue;
        }
        memcpy(static_cast<char*>(dataList) + clientRank * dataLen, singleData, dataLen);
        close(clientSkt);
        ++connectedClientCount;
    }

    free(singleData);
    close(m_serverSkt);
    DEBUG("rank: %d, Server gather stopped after serving %d clients.", m_rankID, m_rankSize - 1);
}

void HcclCommunicater::ClientBcast(
    void *dataBuffer,
    const size_t dataLen
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
    while (true) {
        if (connect(m_clientSkt, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
            sleep(RETRY_INTERVAL);
            continue;
        }
        DEBUG("rank: %d,  connect server success! ", m_rankID);
        if (recv(m_clientSkt, &connectedClientCount, sizeof(int), 0) <= 0) {
            sleep(RETRY_INTERVAL);
            continue;
        }
        break;
    }

    for (int i = 0; i < RETRY_COUNT; i++) {
        if (send(m_clientSkt, &m_rankID, sizeof(int), 0) <= 0) {
            sleep(RETRY_INTERVAL);
        } else {
            break;
        }
    }

    for (int i = 0; i < RETRY_COUNT; i++) {
        if (send(m_clientSkt, static_cast<char*>(dataBuffer), dataLen, 0) <= 0) {
            sleep(RETRY_INTERVAL);
        } else {
            break;
        }
    }
    close(m_clientSkt);
    DEBUG("client rank: %d, broadcast success!", m_rankID);
}

bool HcclCommunicater::ServerPreset()
{
    // 服务器套接字校验
    m_serverSkt = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverSkt == -1) {
        ERROR("rank: %d, create socket failed.", m_rankID);
        return false;
    }
    DEBUG("rank: %d, serverSkt: %d", m_rankID, m_serverSkt);

    int reuse = 1;
    if (setsockopt(m_serverSkt, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        ERROR("rank: %d, setsocket options failed.", m_rankID);
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
        ERROR("rank: %d, create socket failed.", m_rankID);
        return false;
    }
    DEBUG("rank: %d, , clientSkt: %d", m_rankID, m_clientSkt);
    int reuse = 1;
    if (setsockopt(m_clientSkt, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        ERROR("rank: %d, set socket options failed.", m_rankID);
        close(m_clientSkt);
        return false;
    }
    return true;
}

}