#include "defs.h"

void* ss_handle_write(void* arg)
{
    request_t* r = arg;
    message_t msg = r->msg;
    int client_socket = r->sock;

    char* client_ip = (char*)malloc(INET_ADDRSTRLEN*sizeof(char));
    int client_port = ntohs(r->addr.sin_port);
    const char* ret = inet_ntop(AF_INET, &(r->addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    if (ret == NULL) {
        perror_tpx(r, "inet_ntop");
    } 
    // Log the event if client_ip conversion was successful
    logst(EVENT, "Received write request from %s:%d, for %s", client_ip, client_port, msg.path);

    char* path = (char*)malloc(SIZE*sizeof(char));
    char localpath[SIZE] =  {0};
    if(msg.path[0] == '/')
    strcpy(localpath, ".");
    strcat(localpath, msg.path);
    strcat(localpath, "/");
    strcat(localpath, msg.file_name);
    int ac = access(localpath, F_OK);
    if(ac < 0)
    {
        // localpath = 0;
        strcpy(localpath,".bkp/");
        strcat(localpath, msg.path);
    }
    printf("%s\n", localpath);

    FILE *f = fopen(localpath, "w+");
    if (f == NULL)
    {
        perror_tpx(r, "fopen");
        return NULL;
    }

    msg.type = WRITE + 1;
    booking(&msg);
    send_txn(client_socket, &msg, sizeof(message_t), 0, r, 1);
    recv_txn(client_socket, &msg, sizeof(message_t), 0, r, 1);

    int data = atoi(msg.data);
    logst(PROGRESS, "Receiving %d bytes from %s:%d", data, client_ip, client_port);

    for(;1;)
    {
        recv_txn(client_socket, &msg, sizeof(message_t), 0, r, 1);

        if (msg.type == STOP)
        {
            fclose(f);
            goto confirmation;
        }
        else if (msg.type == WRITE && data > SIZE)
        {
            int ret = fwrite(msg.data, sizeof(char), SIZE, f);
            if (ret < 0) {
                perror_tpx(r, "fwrite");
            }
            data = data - SIZE;
            continue;
        }
        else if (msg.type == WRITE && data > 0)
        {
            int ret = fwrite(msg.data, sizeof(char), data, f);
            if (ret < 0) {
                perror_tpx(r, "fwrite");
            }
            // data = data - ret;
            data = 0;
            continue;
        }

    }
    confirmation:

    int name_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (name_socket < 0)
    {
        printf("Unable to create socket\n");
        return NULL;
    }

    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NAME_PORT);
    addr.sin_addr.s_addr = inet_addr(get_local_ip());

    if (connect(name_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Unable to connect to Naming Server");
        return NULL;
    } 

    msg.type = WRITE + 1;
    strcpy(msg.data, path);
    booking(&msg);
    send_txn(name_socket, &msg, sizeof(message_t), 0, r, 1);
    logst(COMPLETION, "Sent write confirmation, for %s, to the naming server", path);
    recv_txn(name_socket, &msg, sizeof(message_t), 0, r, 1);

    struct stat state;
    if (stat(msg.data, &state) < 0) {
        msg.type = UNAVAILABLE;
        booking(&msg);
        send_txn(name_socket, &msg, sizeof(message_t), 0, r, 1);
        reqfree(r);
        return NULL;
    }

    metadata_t info;
    strcpy(info.path, msg.data);
    info.mode = state.st_mode;
    info.size = state.st_size;
    info.ctime = state.st_ctime;
    info.mtime = state.st_mtime;

    msg.type = INFO + 1;
    data = sizeof(metadata_t);
    sprintf(msg.data, "%d", data);
    booking(&msg);
    send_txn(name_socket, &msg, sizeof(message_t), 0, r, 1);
    logst(PROGRESS, "Sending %d bytes of metadata to naming server", data);

    int sent = 0;
    void* p = &info;
    while(sent < data)
    {
        if (data - sent >= SIZE)
        {
            memcpy(msg.data, p + sent, SIZE);
            booking(&msg);
            send_txn(name_socket, &msg, sizeof(message_t), 0, r, 1);
            sent = sent + SIZE;
        }
        else
        {
            memset(msg.data, 0, SIZE);
            memcpy(msg.data, p + sent, data - sent);
            booking(&msg);
            send_txn(name_socket, &msg, sizeof(message_t), 0, r, 1);
            sent = data;
        }
    }

    msg.type = STOP;
    booking(&msg);
    send_txn(name_socket, &msg, sizeof(message_t), 0, r, 1);

    logst(COMPLETION, "Sent %d bytes of metadata to naming server", data);
    reqfree(r);
    return NULL;
}







    

