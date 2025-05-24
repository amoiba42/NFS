#include "client.h"
#include "defs.h"

void request_copy(int name_socket)
{
    message_t msg;
    msg.type = COPY;
    // memset(msg.data, 0, SIZE);
    memset(msg.path, 0, PATH_MAX);

    char *current = (char *)malloc(PATH_MAX * sizeof(char));
    char *new = (char *)malloc(PATH_MAX * sizeof(char));

    printf("Current Path: ");
    scanf(" %[^\n]s", current);
    printf("New Path: ");
    scanf(" %[^\n]s", new);

    strcpy(msg.path, current);
    booking(&msg);
    if (send(name_socket, &msg, sizeof(message_t), 0) < 0)
    {
        perror_tx("send");
        return;
    }

    strcpy(msg.path, new);
    booking(&msg);
    if (send(name_socket, &msg, sizeof(message_t), 0) < 0)
    {
        perror_tx("send");
        return;
    }

    if (recv(name_socket, &msg, sizeof(message_t), 0) < 0)
    {
        perror_tx("recv");
        return;
    }

    if (msg.type == COPY + 1)
    {
        logc(COMPLETION, "%s was copied to %s", current, new);
    }
    else if (msg.type == NOTFOUND)
    {
        logc(FAILURE, "%s was not found", current);
    }
    else if (msg.type == UNAVAILABLE)
    {
        logc(FAILURE, "%s is unavailable currently", current);
    }
    else if (msg.type == XLOCK)
    {
        logc(FAILURE, "%s is being written to by a client", current);
    }
    else if (msg.type == PERM)
    {
        logc(FAILURE, "Missing permissions for copy");
    }
    else
    {
        logc(FAILURE, "Received an invalid response %d from the server", msg.type);
    }
}
