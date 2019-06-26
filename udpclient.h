#pragma once 
#include <thread>
#include <atomic>
#include "udp_message.h"
#include "endpoint.h"

class CUdpClient {
public:
    using RecvMessageCb = std::function<void(endpoint_t from, Message msg)>;
    
    CUdpClient(std::string servIp, uint16_t port);
    virtual ~CUdpClient();
    void SetMessageCb(RecvMessageCb cb);
    void SendText(std::string text);
    int SocketFd() {
        return _connSock;
    }
    endpoint_t &RemoteEndpoint() {
        return _servEndpoint;
    }
    bool Start();
private:
    void OnRecvMsgTh();
private:
    std::string         _servIp;
    uint16_t            _servPort;
    std::atomic<bool>   _shutdown;
    int                 _connSock;

    std::thread         _RecvLoopth;
    endpoint_t          _servEndpoint;
    RecvMessageCb       _messagecb;
};

