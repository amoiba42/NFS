#include "client.h"
#include "defs.h"

void request_create(int name_socket)
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
        msg.type = CREATE_D;
    }
    else if (c == 'F')
    {
        msg.type = CREATE_F;
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

    if (msg.type == CREATE_D + 1 || msg.type == CREATE_F + 1)
    {
        logc(COMPLETION, "%s was created", path);
    }
    else if (msg.type == EXISTS)
    {
        logc(FAILURE, "%s already exists", path);
    }
    else if (msg.type == NOTFOUND)
    {
        logc(FAILURE, "Parent directory of %s not found", path);
    }
    else if (msg.type == PERM)
    {
        logc(FAILURE, "Missing permissions for create");
    }
    else if (msg.type == UNAVAILABLE)
    {
        logc(FAILURE, "Server unavailable");
    }
    else
    {
        logc(FAILURE, "Invalid server response: %d", msg.type);
    }
}