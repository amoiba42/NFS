#include "client.h"
#include "defs.h"

void request_delete(int name_socket)
{
    message_t msg;
    memset(msg.data, 0, SIZE);
    memset(msg.path, 0, PATH_MAX);

    char c;
    printf("[F]ile or [D]irectory: ");
    scanf("%c", &c);
    scanf("%c", &c);
    c = toupper(c);

    if (c == 'D')
    {
        msg.type = DELETE_D;
    }
    else if (c == 'F')
    {
        msg.type = DELETE_F;
    }
    else
    {
        fprintf(stderr, "Invalid request\n");
        return;
    }

    char path[PATH_MAX];
    printf("Path: ");
    scanf(" %[^\n]s", path);
    strcpy(msg.path, path);
    char *file_name = (char *)malloc(SIZE * sizeof(char));
    if (c == 'D')
    {
        printf("Directory: ");
        scanf(" %[^\n]s", file_name);
        strcpy(msg.file_name, file_name);
    }
    else
    {
        printf("File Name: ");
        scanf(" %[^\n]s", file_name);
        strcpy(msg.file_name, file_name);
    }

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

    if (msg.type == DELETE_D + 1 || msg.type == DELETE_F + 1)
    {
        logc(COMPLETION, "%s was deleted", path);
    }
    else if (msg.type == NOTFOUND)
    {
        logc(FAILURE, "%s was not found", path);
    }
    else if (msg.type == UNAVAILABLE)
    {
        logc(FAILURE, "%s is currently unavailable", path);
    }
    else if (msg.type == BEING_READ)
    {
        logc(FAILURE, "%s is currently being read", path);
    }
    else if (msg.type == RDONLY)
    {
        logc(FAILURE, "%s is read-only", path);
    }
    else if (msg.type == XLOCK)
    {
        logc(FAILURE, "%s is being written to by a client", path);
    }
    else if (msg.type == PERM)
    {
        logc(FAILURE, "Missing permissions for delete");
    }
    else
    {
        logc(FAILURE, "Invalid server response: %d", msg.type);
    }
}