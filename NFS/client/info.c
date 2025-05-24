#include "client.h"
#include "defs.h"

void request_info(int name_socket)
{
    message_t msg;
    msg.type = INFO;
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

    if (msg.type == INFO + 1)
    {
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
    if (connect(ss_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror_tx("Unable to connect to Storage Server");
        return;
    }

    msg.type = INFO;
    strcpy(msg.path, path);
    booking(&msg);
    if (send(ss_sock, &msg, sizeof(message_t), 0) < 0)
    {
        return;
    }
    if (recv(ss_sock, &msg, sizeof(message_t), 0) < 0)
    {
        return;
    }
    if (msg.type == STOP)
    {
        logc(COMPLETION, "Received info from storage server %s:%d", ip, port);  
        printf("%s\n", msg.data);  
    }
    else
    {
        logc(FAILURE, "Received an invalid response %d from the server", msg.type);
    }

    free(path);
    free(ip);
    close(ss_sock);
}