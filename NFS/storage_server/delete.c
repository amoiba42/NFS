#include "ss.h"

void* ss_handle_delete(void* arg, int flag)
{
    request_t* r = arg;
    message_t msg = r->msg;
    int sock = r->sock;
    char path[1024] = {0};
if(msg.path[0] == '/')
    strcpy(path, ".");
    strcat(path, msg.path);
    strcat(path, "/");
    strcat(path, msg.file_name);

    if (remove(msg.path) < 0)
    {
        char path[SIZE] = {0};
        strcat(path,".bkp/");
        strcat(path, msg.path);
        remove(path);
    }

    if (flag)
    {
         msg.type = DELETE_D + 1;
    }
    else
    {
         msg.type = DELETE_F + 1;
    }
     booking(&msg);
    send_txn(sock, &msg, sizeof(msg), 0, r,1);

    reqfree(r);
    return NULL;
}