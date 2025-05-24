#include "defs.h"

void* ss_handle_info(void* arg)
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

    logst(EVENT, "Received info request from %s:%d, for %s", client_ip, client_port, msg.file_name);

    // char fullpath[PATH_MAX];
    // char result[PATH_MAX];

    // perror("2");

    // char *pos = strchr(msg.path, '/');

    // if (pos == msg.path)
    // {
    //     size_t length = pos - msg.path;
    //     strcpy(result, ".");
    //     result[length] = '\0';
    // } 
    // else 
    // {
    //     strcpy(result, msg.path);
    // }

    // strcat(fullpath, result);
    // strcat(fullpath, "/");
    // strcat(fullpath, msg.file_name);

    // printf("%s\n", fullpath);

    // perror("3");

    struct stat file_stat;
    char buffer[BUFSIZE+1000];

    if (stat(msg.file_name, &file_stat) == 0) 
    {
        char access_time[32], mod_time[32], change_time[32];
        strftime(access_time, sizeof(access_time), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_atime));
        strftime(mod_time, sizeof(mod_time), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime));
        strftime(change_time, sizeof(change_time), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_ctime));

        snprintf(buffer, sizeof(buffer),
                 "Size: %ld bytes\n"
                 "Permissions: %o (octal)\n"
                 "Last Access: %s\n"
                 "Last Modification: %s\n"
                 "Last Status Change: %s\n",
                 file_stat.st_size,
                 file_stat.st_mode & 0777,
                 access_time,
                 mod_time,
                 change_time);

        strcpy(msg.data, buffer);
    } 
    else 
    {
        perror("stat failed");
    }

    // msg.type = INFO + 1;
    // send_txn(client_socket, &msg, sizeof(msg), 0, r, 1);
    logst(PROGRESS, "Sending metadata to %s:%d", client_ip, client_port);

    msg.type = STOP;
    send_txn(client_socket, &msg, sizeof(msg), 0, r, 1);

    int ns_socket = socket_txn(AF_INET, SOCK_STREAM, 0, r, 1);
    r->newsock = ns_socket;

    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NAME_PORT);
    addr.sin_addr.s_addr = inet_addr(get_local_ip());

    connect_t(ns_socket, (struct sockaddr*) &addr, sizeof(addr));

    msg.type = INFO + 1;
    send_txn(ns_socket, &msg, sizeof(msg), 0, r, 1);

    logst(COMPLETION, "Sent metadata confirmation for %s to naming server", msg.file_name);
    
    reqfree(r);
    return NULL;
}