#ifndef UDP_MESSAGE_H
#define UDP_MESSAGE_H

#include <stdint.h>
#include "endpoint.h"

#define MSG_MAGIC 0x8964 
#define MSG_MAGICLEN 2
#define MSG_TYPELEN 2
#define MSG_BODYLEN 4
#define MSG_HEADLEN MSG_MAGICLEN + MSG_TYPELEN + MSG_BODYLEN
/* a message is a UDP datagram with following structure:
   -----16bits--+---16bits--+-----32bits----------+---len*8bits---+
   --  0x8964   + msg type  + msg length(exclude) + message body  +
   -------------+-----------+---------------------+---------------+
*/
#define SEND_BUFSIZE 1024
#define RECV_BUFSIZE 1024

enum _MessageType {
    MTYPE_LOGIN = 0,
    MTYPE_LOGOUT,
    MTYPE_LIST,
    MTYPE_PUNCH,
    MTYPE_PING,
    MTYPE_PONG,
    MTYPE_REPLY,
    MTYPE_TEXT,
    MTYPE_END
};
typedef enum _MessageType MessageType;

struct _MessageHead {
    uint16_t magic;
    uint16_t type;
    uint32_t length;
}__attribute__((packed));
typedef struct _MessageHead MessageHead;

struct _Message {
    MessageHead head;
    const char *body;
};
typedef struct _Message Message;

class CMessage {
public:
    static const char *strmtype(MessageType type) {
        switch(type) {
            case MTYPE_LOGIN:   return "LOGIN";
            case MTYPE_LOGOUT:  return "LOGOUT";
            case MTYPE_LIST:    return "LIST";
            case MTYPE_PUNCH:   return "PUNCH";
            case MTYPE_PING:    return "PING";
            case MTYPE_PONG:    return "PONG";
            case MTYPE_REPLY:   return "REPLY";
            case MTYPE_TEXT:    return "TEXT";
            default:            return "UNKNOW";
        }
    }
    static int msg_pack(Message msg, char *buf, unsigned int bufsize) {
        if (bufsize < MSG_HEADLEN + msg.head.length) {
            printf("buf too small");
            return 0;
        }
        int16_t m_magic = htons(msg.head.magic);
        int16_t m_type = htons(msg.head.type);
        int32_t m_length = htonl(msg.head.length);
        int index = 0;
        memcpy(buf + index, &m_magic, MSG_MAGICLEN);
        index += MSG_MAGICLEN;
        memcpy(buf + index, &m_type, MSG_TYPELEN);
        index += MSG_TYPELEN;
        memcpy(buf + index, &m_length, MSG_BODYLEN);
        index += MSG_BODYLEN;
        memcpy(buf + index, msg.body, msg.head.length);
        index += msg.head.length;
        return index;
    }
    static Message msg_unpack(const char *buf, unsigned int buflen) {
        Message m;
        memset(&m, 0, sizeof(m));
        if (buflen < MSG_HEADLEN) {
            // at least we won't get an overflow
            return m;
        }
        int index = 0;
        m.head.magic = ntohs(*(uint16_t *)(buf + index));
        index += sizeof(uint16_t);
        if (m.head.magic != MSG_MAGIC) {
            return m;
        }
        m.head.type = ntohs(*(uint16_t *)(buf + index));
        index += sizeof(uint16_t);
        m.head.length = ntohl(*(uint32_t *)(buf + index));
        index += sizeof(uint32_t);
        if (index + m.head.length > buflen) {
            printf("message declared body size(%d) is larger than what's received (%d), truncating\n",
                    m.head.length, buflen - MSG_HEADLEN);
            m.head.length = buflen - index;
        }
        m.body = buf + index;
        return m;
    }

    // replay a Message
    static int udp_send_msg(int sock, endpoint_t peer, Message msg) {
        char buf[SEND_BUFSIZE] = {0};
        int wt_size = msg_pack(msg, buf, SEND_BUFSIZE);
        return sendto(sock, buf, wt_size,
                MSG_DONTWAIT, (struct sockaddr *)&peer, sizeof(peer));
    }
    
    // reply a buf with length
    static int udp_send_buf(int sock, endpoint_t peer, MessageType type,
            const char *buf, unsigned int len) {
        Message m;
        m.head.magic = MSG_MAGIC;
        m.head.type = type;
        m.head.length = len;
        m.body = buf;
        return udp_send_msg(sock, peer, m);
    }
    // reply a NULL terminated text
    static int udp_send_text(int sock, endpoint_t peer, MessageType type, const char *text) {
        unsigned int len = text == NULL ? 0 : strlen(text);
        return udp_send_buf(sock, peer, type, text, len);
    }
};

#endif
