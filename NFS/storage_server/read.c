#include "defs.h"
extern char PATH[SIZE];
void* ss_handle_read(void* arg)
{
    request_t* r = arg;
    message_t msg = r->msg;
    int client_socket = r->sock;

    char* client_ip = (char*)malloc(INET_ADDRSTRLEN*sizeof(char));
    int client_port = ntohs(r->addr.sin_port);
    const char* ret = inet_ntop(AF_INET, &(r->addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    if (ret == NULL) 
    {
        perror_tpx(r, "inet_ntop");
    } 

    logst(EVENT, "Received read request from %s:%d, for %s", client_ip, client_port, msg.path);

    char orgpath[SIZE];
    strcpy(orgpath, msg.path);

    char path[SIZE] = {0};
    if(msg.path[0] == '/')
    strcpy(path, ".");
    strcat(path, msg.path);
    strcat(path, "/");
    strcat(path, msg.file_name);

    printf("%s\n",path);
    // int ac = access(path, F_OK);
    // if (ac != 0){
    //     bzero(path, SIZE*sizeof(char));
    //     strcat(path,".bkp/");
    //     strcat(path, msg.path);
    //     remove(path);
    // }
    // printf("%s\n",path);
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        perror_tpx(r, "fopen");
        return NULL;
    };

    struct stat st;
    if (stat(path, &st) < 0)
    {
        msg.type = PERM;
        send_txn(client_socket, &msg, sizeof(msg), 0, r, 1);
        logst(FAILURE, "Returning read request from %s:%d, due to lack of permissions", client_ip, client_port);
    }
    else
    {
        int bytes = st.st_size;
        msg.type = READ + 1;
        sprintf(msg.data, "%d", bytes);
        send_txn(client_socket, &msg, sizeof(msg), 0, r, 1);
        logst(PROGRESS, "Sending %d bytes to %s:%d", bytes, client_ip, client_port);

        int sent = 0;
        while (sent < bytes)
        {
            if (bytes - sent >= SIZE)
            {
                int ret = fread(msg.data, SIZE*sizeof(char), SIZE, f);
                if (ret != SIZE) 
                {
                    perror_tpx(r, "fread");
                }
                send_txn(client_socket, &msg, sizeof(msg), 0, r, 1);
                sent += SIZE;
            }
            else
            {
                memset(msg.data, 0, SIZE);
                int ret = fread(msg.data, sizeof(char)* (bytes - sent), bytes - sent, f);
                if (ret < 0) 
                {
                    perror_tpx(r, "fread");
                }
                send_txn(client_socket, &msg, sizeof(msg), 0, r, 1);
                sent = bytes;
            }
        }

        msg.type = STOP;
        send_txn(client_socket, &msg, sizeof(msg), 0, r, 1);
    }
    fclose(f);
    int ns_socket = socket_txn(AF_INET, SOCK_STREAM, 0, r, 1);
    r->newsock = ns_socket;

    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NAME_PORT);
    addr.sin_addr.s_addr = inet_addr(get_local_ip());

    connect_t(ns_socket, (struct sockaddr*) &addr, sizeof(addr));

    msg.type = READ + 1;
    strcpy(msg.path, orgpath);
    send_txn(ns_socket, &msg, sizeof(msg), 0, r, 1);

    logst(COMPLETION, "Sent read confirmation, for %s, to naming server", path);
    
    reqfree(r);
    return NULL;
}