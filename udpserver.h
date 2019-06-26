#pragma once 

#include <thread>
#include <atomic>
#include <mutex>

#include "stub_message.h"
#include "endpoint.h"

class CUdpServer {
public:
    using MessageRecvCb = std::function<void(int listensock, endpoint_t peer, Message msg)>;
    CUdpServer() = delete;
    //CUdpServer & operator=(CUdpServer &) = delete;
    CUdpServer(std::string host, uint16_t port, MessageRecvCb cb) {
        _shutdown = false;
        _host = host;
        _port = port;
        _msgcb = cb;
    }
    ~CUdpServer() {
        _shutdown = true;
        if (_loopth.joinable()) {
            _loopth.join();
        }
    }
    bool Start();
private:
    void workLoop(int listensock);
private:
    std::string         _host;
    uint16_t            _port;
    std::atomic<bool>   _shutdown;
    std::thread         _loopth;
    MessageRecvCb       _msgcb;
    std::mutex          _mutex;
};
