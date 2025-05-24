#include "client.h"
#include "defs.h"

void request_list(int name_socket)
{
    message_t msg;
    msg.type = LIST;
    ssize_t bytes = 0;
    booking(&msg);
    send_txn(name_socket, &msg, sizeof(msg), 0, NULL, 0);
    // recv_txn(name_socket, &msg, sizeof(msg), 0, NULL,0);

    while (1)
    {
        bytes += recv_txn(name_socket, &msg, sizeof(msg), 0, NULL, 0);
        // recv_tx(sock, &msg, sizeof(msg), 0);
        switch (msg.type)
        {
        case STOP:
            logc(COMPLETION, "Received %ld bytes from naming server", bytes);
            return;
            break;

        case LIST + 1:
            printf("%s\n", msg.data);
            break;
        default:
            logc(FAILURE, "Received an invalid response %d from the server", msg.type);
            return;
        }
    }
}
