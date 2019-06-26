#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unordered_map>
#include <functional>
#include <string>
#include "endpoint.h"
#include "udpserver.h"

std::unordered_map<std::string, int> g_sessionlist;

bool CUdpServer::Start()
{
    int ret;
    endpoint_t server = Endpoint::ep_frompair(_host.c_str(), _port);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        return false;
    }
    ret = bind(sock, (const struct sockaddr*)&server, sizeof(server));
    if (ret == -1) { 
        perror("bind");
        return false;
    }

    printf("server start on %s \n", Endpoint::ep_tostring(server));

    _loopth = std::thread(&CUdpServer::workLoop, this, sock);
    return true;
}

void CUdpServer::workLoop(int listen_sock)
{    
    endpoint_t peer;
    socklen_t addrlen;
    char buf[RECV_BUFSIZE];
    while (!_shutdown) {
        addrlen = sizeof(peer);
        memset(&peer, 0, addrlen);
        memset(buf, 0, RECV_BUFSIZE);
        int rd_size;
        /* UDP isn't a "stream" protocol. once you do the initial recvfrom,
           the remainder of the packet is discarded */
        rd_size = recvfrom(listen_sock, buf, RECV_BUFSIZE, 0,
                (struct sockaddr*)&peer, &addrlen);
        if (rd_size == -1) {
            perror("recvfrom");
            break;
        } else if (rd_size == 0) {
            printf("EOF from %s", Endpoint::ep_tostring(peer));
            continue;
        }
        Message msg = CMessage::msg_unpack(buf, rd_size);
        if (msg.head.magic != MSG_MAGIC || msg.body == NULL) {
            printf("Invalid message(%d bytes): {0x%x,%d,%d} %p", rd_size,
                    msg.head.magic, msg.head.type, msg.head.length, msg.body);
            continue;
        }
        if (_msgcb) {
            _msgcb(listen_sock, peer, msg);
        }
        
        continue;

    }
    close(listen_sock);
    printf("udp_receive_loop exit! \n");
}

int main()
{
    CUdpServer udpsvr("127.0.0.1", 9090, [&](int listensock, endpoint_t peer, Message msg) {
        printf("msgtype:%d, text:%s \n", msg.head.type, msg.body);
    });
    if (!udpsvr.Start()) {
        std::cout << "start server fail! " << std::endl;
        return -1;
    }
    getchar();
    return 0;
}