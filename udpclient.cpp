#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "udpclient.h"

CUdpClient::CUdpClient(std::string servIp, uint16_t port)
{
     _servIp = servIp;
     _servPort = port;
     _shutdown = false;
}

CUdpClient::~CUdpClient()
{
    _shutdown = true;

    if (_connSock > 0) {::close(_connSock);}
}

bool CUdpClient::Start()
{
    _connSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (_connSock == -1) {
        perror("socket");
        return false;
    }
    _servEndpoint = Endpoint::ep_frompair(_servIp.c_str(), _servPort);
    
    return true;
}

void CUdpClient::SendText(std::string text)
{
    printf("send text:%s\n", text.c_str());
    CMessage::udp_send_text(SocketFd(), _servEndpoint, MTYPE_TEXT, (const char *)text.c_str());
}

void CUdpClient::SetMessageCb(RecvMessageCb cb)
{
    _messagecb = cb;    
    _RecvLoopth = std::thread(&CUdpClient::OnRecvMsgTh, this);
}

void CUdpClient::OnRecvMsgTh()
{
    printf("````recv th running ```\n");
    endpoint_t peer;
    socklen_t addrlen;
    char buf[RECV_BUFSIZE];
    int nfds;
    fd_set readfds;
    struct timeval timeout;

    nfds = _connSock + 1;
    while(!_shutdown) {
        FD_ZERO(&readfds);
        FD_SET(_connSock, &readfds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int ret = select(nfds, &readfds, NULL, NULL, &timeout);
        if (ret == 0) {
            /* timeout */
            continue;
        }
        else if (ret == -1) {
            perror("select");
            continue;
        }
        assert(FD_ISSET(_connSock, &readfds));
        addrlen = sizeof(peer);
        memset(&peer, 0, addrlen);
        memset(buf, 0, RECV_BUFSIZE);
        int rd_size = recvfrom(_connSock, buf, RECV_BUFSIZE, 0,
                (struct sockaddr*)&peer, &addrlen);
        if (rd_size == -1) {
            perror("recvfrom");
            continue;
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
        if (_messagecb) {
            _messagecb(peer, msg);
        }       
    }
    printf("quiting receive_loop");
    return ;
}

int main()
{
    CUdpClient udpcli("127.0.0.1", 9090);
    udpcli.Start();
    udpcli.SendText("hello world!");
    
    return 0;
}
