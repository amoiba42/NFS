#include "defs.h"


void confirm_stream_to_ns(const char *stream_path, const char *ns_ip, uint16_t ns_port, request_t *req)
{
    int ns_socket = socket_txn(AF_INET, SOCK_STREAM, 0, req, 1);
    if (ns_socket < 0)
    {
        perror_tx("socket_txn");
        return;
    }

    req->newsock = ns_socket;
    printf("inside confirm to ns\n");

    struct sockaddr_in* addr_ptr = malloc(sizeof(struct sockaddr_in));
    struct sockaddr_in addr = *addr_ptr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ns_port);
    addr.sin_addr.s_addr = inet_addr(ns_ip);

    printf("inside confirm to ns\n");
    if (connect_t(ns_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror_tx("connect_t");
        close(ns_socket);
        return;
    }

    message_t msg;
    msg.type = STREAM + 1;
    printf("inside confirm to ns\n");
    strcpy(msg.path, stream_path);
    msg.path[sizeof(msg.path) - 1] = '\0';

    printf("inside confirm to ns\n");
    booking(&msg);
    if (send_txn(ns_socket, &msg, sizeof(msg), 0, req, 1) < 0)
    {
        perror_tx("send_txn");
        close(ns_socket);
        return;
    }

    printf("inside confirm to ns\n");
    logst(COMPLETION, "Sent STREAM confirmation for %s to naming server at %s:%d",
          stream_path, ns_ip, ns_port);

    close(ns_socket);
}

void serve_audio(int client_socket, const char *audio_file_path)
{
    FILE *audio_file = fopen(audio_file_path, "rb");
    if (!audio_file)
    {
        perror_tx("");
        close(client_socket);
        return;
    }

    char buffer[SIZE];
    ssize_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), audio_file)) > 0)
    {
        send_txn(client_socket, buffer, bytes_read, 0, NULL, 0);
    }

    fclose(audio_file);
    close(client_socket);
    
    // printf("Audio streaming completed.\n");
}

void *ss_handle_audio(void *arg)
{
    request_t *req = arg;
    message_t msg = req->msg;
    int client_socket = req->sock;

    char *client_ip = (char *)malloc(INET_ADDRSTRLEN * sizeof(char));
    int client_port = ntohs(req->addr.sin_port);
    const char *ret = inet_ntop(AF_INET, &(req->addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    if (ret == NULL)
    {
        perror_tx("inet_ntop");
    }

    logst(EVENT, "Received stream request from %s:%d, for %s", client_ip, client_port, msg.data);

    char orgpath[SIZE];
    strcpy(orgpath, msg.path);
    char path[SIZE] = {0};
    strcat(path, msg.file_name);
    printf("%s\n", path);

    int ac = access(path, F_OK);
    if (ac != 0)
    {
        logst(FAILURE, "File %s not found", path);
        msg.type = PERM;
        booking(&msg);
        send_txn(client_socket, &msg, sizeof(msg), 0, req, 1);
        return NULL;
    }

    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        perror_tpx(req, "fopen");
        return NULL;
    }

    printf("opened file\n");
    struct stat st;
    if (stat(path, &st) < 0)
    {
        msg.type = PERM;
        booking(&msg);
        send_txn(client_socket, &msg, sizeof(msg), 0, req, 1);
        logst(FAILURE, "Returning stream request from %s:%d, due to lack of permissions", client_ip, client_port);
        reqfree(req);
        return NULL;
    }
    else
    {
        int bytes = st.st_size;
        msg.type = STREAM + 1;
        sprintf(msg.data, "%d", bytes);
booking(&msg);
        if (send_txn(client_socket, &msg, sizeof(msg), 0, req, 1) < 0)
        {
            logst(FAILURE, "Failed to send file size", client_ip, client_port);
            return NULL;
        }
        logst(PROGRESS, "Streaming %d bytes of audio to %s:%d", bytes, client_ip, client_port);

    char buffer[SIZE];
    ssize_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0)
    {
        send_txn(client_socket, buffer, bytes_read, 0, NULL, 0);
    }
    }
    fclose(f);
    close(client_socket);
    printf("completed streaming\n");
    // logst(PROGRESS, "Completed streaming %d bytes of audio data", sent, client_ip, client_port);
    confirm_stream_to_ns(orgpath, get_local_ip(), NAME_PORT, req);
    reqfree(req);
    return NULL;
}