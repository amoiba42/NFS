#include "client.h"
#include "defs.h"

void request_read(int name_socket)
{
    message_t msg;
    msg.type = READ;
    memset(msg.path, 0, PATH_MAX);
    memset(msg.data, 0, SIZE);

    char *path = (char *)malloc(SIZE * sizeof(char));
    printf("Path: ");
    scanf(" %[^\n]s", path);
    strcpy(msg.path, path);

    char *file_name = (char *)malloc(SIZE * sizeof(char));
    printf("File: ");
    scanf(" %[^\n]s", file_name);
    strcpy(msg.file_name, file_name);

    char *localpath = (char *)malloc(SIZE * sizeof(char));
    printf("Local Path: ");
    scanf(" %[^\n]s", localpath);

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

    char *ip = (char *)malloc(INET_ADDRSTRLEN * sizeof(char));
    int port;
    int flag_bck = 0;
    // printf("%d\n", msg.type);
    if (msg.type == READ + 1)
    {
        if(strcmp(msg.data, "Backup") == 0)
        {
            flag_bck = 1;
        }
        strncpy(ip, msg.send_ip.ip, INET_ADDRSTRLEN);
        port = msg.send_ip.port;

        logc(PROGRESS, "Sending request to storage server %s:%d", ip, port);
    }
    else if (msg.type == NOTFOUND)
    {
        logc(FAILURE, "%s was not found", path);
        return;
    }
    else if (msg.type == UNAVAILABLE)
    {
        logc(FAILURE, "%s is unavailable currently", path);
        return;
    }
    else if (msg.type == XLOCK)
    {
        logc(FAILURE, "%s is being written to by a client", path);
        return;
    }
    else if (msg.type == PERM)
    {
        logc(FAILURE, "Missing permissions for read");
        return;
    }
    else
    {
        logc(FAILURE, "Received an invalid response %d from the server", msg.type);
        return;
    }

    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    int ss_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (ss_sock < 0)
    {
        perror_tx("sock");
        return;
    }
    // printf("%s:%d\n",ip,port);
    if (connect(ss_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror_tx("Unable to connect to Storage Server");
        return;
    }

    msg.type = READ;
    if(flag_bck == 1)
    {
        sprintf(msg.path,"/bkp%s",msg.path);
    }

    strcpy(msg.path, path);
    booking(&msg);
    if (send(ss_sock, &msg, sizeof(message_t), 0) < 0)
    {
        perror_tx("send");
        return;
    }
    if (recv(ss_sock, &msg, sizeof(message_t), 0) < 0)
    {
        perror_tx("recv");
        return;
    }

    int bytes;
    FILE *file = fopen(localpath, "w");
    if (file == NULL)
    {
        perror_tx("fopen");
        return;
    }

    if (msg.type == READ + 1)
    {
        bytes = atoi(msg.data);
        logc(PROGRESS, "Receiving %d bytes from storage server %s:%d", bytes, ip, port);
    }
    else if (msg.type == NOTFOUND)
    {
        logc(FAILURE, "%s was not found", path);
        goto ret;
    }
    else if (msg.type == PERM)
    {
        logc(FAILURE, "Missing permissions for read");
        goto ret;
    }
    else
    {
        logc(FAILURE, "Invalid server response: %d", msg.type);
        goto ret;
    }

    int remaining = bytes;
    // printf("%d\n", remaining);
    while (1)
    {
        recv(ss_sock, &msg, sizeof(msg), 0);
        if (msg.type == STOP)
        {
            logc(COMPLETION, "Received %d bytes from storage server %s:%d", bytes, ip, port);
            break;
        }
        else if (msg.type == READ + 1)
        {
            if (remaining > SIZE)
            {
                fwrite(msg.data, sizeof(char), SIZE, file);
                remaining -= SIZE;
                continue;
            }
            else if (remaining > 0)
            {
                fwrite(msg.data, sizeof(char), remaining, file);
                remaining = 0;
                continue;
            }
        }
        else
        {
            logc(FAILURE, "Received an invalid response %d from the server", msg.type);
        }
    }

ret:
    fclose(file);
    free(path);
    free(localpath);
    free(ip);
    close(ss_sock);
}