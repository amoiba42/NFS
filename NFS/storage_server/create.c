#include "defs.h"

void send_metadata(int sock, metadata_t *mdata, request_t *req)
{
    message_t msg = req->msg;
    int bytes = sizeof(metadata_t);
    sprintf(msg.data, "%ld", sizeof(metadata_t));
    booking(&msg);
    send_txn(sock, &msg, sizeof(msg), 0, req, 1);

    logst(PROGRESS, "Sending %d bytes of metadata to naming server", bytes);

    int sent = 0;
    void *ptr = mdata;

    while (sent < bytes)
    {
        if (bytes - sent >= SIZE)
        {
            memcpy(msg.data, (char *)ptr + sent, SIZE);
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, req, 1);
            sent += SIZE;
        }
        else
        {
            bzero(msg.data, SIZE);
            memcpy(msg.data, (char *)ptr + sent, bytes - sent);
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, req, 1);
            sent = bytes;
        }
    }

    msg.type = STOP;
    booking(&msg);
    send_txn(sock, &msg, sizeof(msg), 0, req, 1);
    logst(COMPLETION, "Sent %d bytes of metadata to naming server", bytes);
    return;
}

void *ss_handle_create(void *arg, int flag)
{
    request_t *req = arg;
    message_t msg = req->msg;
    int sock = req->sock;

    logst(EVENT, "Received create %s request from naming server for %s",
          flag ? "directory" : "file", msg.file_name);

    metadata_t mdata;
    memset(&mdata, 0, sizeof(metadata_t));
    char path[SIZE] = {0};
    if(msg.path[0] == '/')
    strcpy(path, ".");
    strcat(path, msg.path);
    strcat(path, "/");
    strcat(path, msg.file_name);
    if (flag)
    {
        // Handle directory creation
        struct stat dir_stat;
        if (stat(path, &dir_stat) < 0)
        {
            if ((mkdir(path, 0777) < 0) || (stat(path, &dir_stat) < 0))
            {
                msg.type = UNAVAILABLE;
                booking(&msg);
                send_txn(sock, &msg, sizeof(msg), 0, req, 1);
                reqfree(req);
                return NULL;
            }
        }

        mdata.mode = dir_stat.st_mode;
        mdata.size = dir_stat.st_size;
        mdata.ctime = dir_stat.st_ctime;
        mdata.mtime = dir_stat.st_mtime;

        logst(COMPLETION, "Created directory %s", msg.path);
    }
    else
    {
        // Handle file creation
        FILE *f = fopen(path, "w");
        if (f == NULL)
        {
            msg.type = UNAVAILABLE;
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, req, 1);
            reqfree(req);
            return NULL;
        }
        fclose(f);

        struct stat file_stat;
        if (stat(msg.file_name, &file_stat) < 0)
        {
            msg.type = UNAVAILABLE;
            booking(&msg);
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, req, 1);
            reqfree(req);
            return NULL;
        }

        mdata.mode = file_stat.st_mode;
        mdata.size = file_stat.st_size;
        mdata.ctime = file_stat.st_ctime;
        mdata.mtime = file_stat.st_mtime;

        logst(COMPLETION, "Created file %s", msg.data);
    }

    strncpy(mdata.path, msg.path, sizeof(mdata.path) - 1);
    mdata.path[sizeof(mdata.path) - 1] = '\0';

    req->msg.type = flag ? CREATE_D + 1 : CREATE_F + 1;
    send_metadata(sock, &mdata, req);

    reqfree(req);
    return NULL;
}
