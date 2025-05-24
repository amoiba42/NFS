#include "defs.h"
extern char PATH[SIZE];

void *ss_handle_ping(void *arg)
{
    request_t *r = arg;
    message_t msg = r->msg;
    // int socket = r->sock;

    char *ip = (char *)malloc(INET_ADDRSTRLEN * sizeof(char));
    int port = ntohs(r->addr.sin_port);
    const char *ret = inet_ntop(AF_INET, &(r->addr.sin_addr), ip, INET_ADDRSTRLEN);
    if (ret == NULL)
    {
        perror_tpx(r, "inet_ntop");
    }

    logst(EVENT, "Received ping request from %s:%d, for %s", ip, port, msg.path);

    msg.type = PING + 1;
    booking(&msg);
    if (send_txn(r->sock, &msg, sizeof(msg), 0,r,1) < 0)
    {
        logst(FAILURE, "handle_ping: Failed to send ping acknowledgement to %s:%d", ip, port);
        logst(EVENT, "This server will be marked down");
        reqfree(r);
        return NULL;
    }

    logst(COMPLETION, "Sent ping acknowledgement to %s:%d", ip,port);

    reqfree(r);
    return NULL;
}