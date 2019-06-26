#ifndef P2PCHAT_ENDPOINT_H
#define P2PCHAT_ENDPOINT_H
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define INET_PORTSTRLEN 5
#define TUPLE_LEN (INET_ADDRSTRLEN + INET_PORTSTRLEN + 1)
typedef struct sockaddr_in endpoint_t;

class Endpoint {
public:
    static bool ep_equal(endpoint_t lp, endpoint_t rp)
    {
        return ( (lp.sin_family == rp.sin_family) &&
        (lp.sin_addr.s_addr == rp.sin_addr.s_addr) &&
        (lp.sin_port == rp.sin_port) ) ? true : false;
    }
    
    /* string is host:port format */
    /* IPV4 ONLY */
    static char *ep_tostring(endpoint_t ep) {
        static char tuple[TUPLE_LEN];
        snprintf(tuple, TUPLE_LEN, "%s:%d",
                inet_ntoa(ep.sin_addr),
                ntohs(ep.sin_port));
        return tuple;
    }
    static endpoint_t ep_fromstring(const char *tuple) {
        char _tuple[TUPLE_LEN];
        const char *host = NULL;
        const char *port = NULL;
        sprintf(_tuple, "%s", tuple);
        host = strtok(_tuple, ":");
        port = strtok(NULL, ":");
        if (host == NULL || port == NULL) {
            host = "255.255.255.255";
            port = "0";
        }
        return ep_frompair(host, atoi(port));
    }
    static endpoint_t ep_frompair(const char *host, short port) {
        endpoint_t ep;
        memset(&ep, 0, sizeof ep);
        ep.sin_family = AF_INET;
        ep.sin_addr.s_addr = inet_addr(host);
        ep.sin_port = htons(port);
        return ep;
    }
};
#endif
