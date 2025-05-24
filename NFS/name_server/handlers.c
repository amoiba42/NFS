#include "ns.h"

extern logfile_t *logfile;
extern d_triesnode *root_trie;
extern d_node *root_map;
extern storage_t *server_registry;
extern LRUCache cache;
extern int ss_count;
extern pthread_mutex_t registry_lock;

unsigned long random_val()
{
    static unsigned long a = 73;
    static unsigned long m = 37;
    static unsigned long c = 109;
    a = a * m + c;
    return a;
}
// Returns if the path exists or not
d_node *is_valid_path(d_node *root, const char *path)
{
    char temp_path[PATH_MAX] = {0};
    strcpy(temp_path, path);
    // temp_path[strlen(path) - 1] = '\0';

    char *token = strtok(temp_path, "/");
    d_node *current = root;

    printf("name: %s\n", path);
    printf("name: %s\n", token);
    while (token)
    {
        int k = sumsqASCII(token);
        if(k < 0)
        { 
            k += MAX_FILES;
        }
        printf("%d\n", k);
        if (current->directories[k])
        {
            printf("name: %sq\n", current->directories[k]->name->name);
            d_node *temp = current->directories[k];
            while (temp)
            {
                printf("name: %sq\n", temp->name->name);
                printf("name: %sq\n", token);
                if (strcmp(temp->name->name, token) == 0)
                {
                    current = temp;
                    return current;
                    break;
                }
                temp = temp->next;
            }
            if (temp == NULL)
                return NULL;
        }
        else
        {
            printf("nulla berozgaar\n");
            return NULL;
        }
        token = strtok(NULL, "/");
    }
    // if(current->files[sumsqASCII(file)]) return true;

    return current;
}

f_node *is_file(f_node **files, const char *token)
{
    if (!files || !token)
    {
        return NULL; // Invalid input
    }

    f_node *current = files[sumsqASCII(token)]; // Access bucket for the given token

    // Traverse the linked list in the bucket
    while (current)
    {
        if (strcmp(current->name->name, token) == 0)
        {                   // Compare names
            return current; // Match found
        }
        current = current->next; // Move to the next node
    }

    return NULL; // No match found
}

void *ns_create_d(void *arg)
{
    request_t *r = arg;
    message_t msg = r->msg;
    int sock = r->sock;

    char ip[INET_ADDRSTRLEN];
    int port = ntohs(r->addr.sin_port);
    const char *ret = inet_ntop(AF_INET, &(r->addr.sin_addr), ip, INET_ADDRSTRLEN);
    if (ret == NULL)
    {
        perror_tpx(r, "inet_ntop");
    }

    logns(EVENT, "Received create directory request from %s:%d, for %s", ip, port, msg.path);

    char temppath[PATH_MAX];
    strcat(temppath, msg.path);
    strcat(temppath, "/");
    strcat(temppath, msg.file_name);

    d_node *temp;
    d_node *d = get(&cache, msg.path);

    if (strcmp(msg.path, "home") == 0)
    {
        temp = root_map;
    }
    else if (d == NULL)
    {
        printf("%s\n", msg.path);
        temp = is_valid_path(root_map, msg.path);
        if (temp == NULL)
        {
            logns(FAILURE, "Returning create directory request from %s:%d, due to invalid path", ip, port);
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, r, 1);
            reqfree(r);
            return NULL;
        }
        d = is_valid_path(root_map, temppath);
        // if(d != NULL)
        //     put(&cache,f->name->name,(void*)f);
    }

    storage_t *ssn = 0;
    char ss_ip[INET_ADDRSTRLEN];
    int ss_sock;
    int ss_port;

    if (d != NULL)
    {
        msg.type = EXISTS;
        logns(FAILURE, "Returning create directory request from %s:%d, due to collision", ip, port);
        booking(&msg);
        send_txn(sock, &msg, sizeof(msg), 0, r, 1);
        reqfree(r);
        return NULL;
    }

    // char parent[PATH_MAX];
    // get_parent_dir(parent, msg.data);

    // f = get(&cache, msg.path);

    // if (f == NULL)
    // {
    //     temp = is_valid_path(root_map, msg.path);
    //     f = is_file(temp->files, msg.file_name);
    //     if(f != NULL)
    //         put(&cache,f->name->name,(void*)f);
    // }

    // if (f == NULL)
    // {
    //     msg.type = NOTFOUND;
    //     logns(FAILURE, "Returning create directory request from %s:%d, for unknown parent directory %s", ip, port, parent);
    //     send_txn(sock, &msg, sizeof(message_t), 0, r, 1);
    //     reqfree(r);
    //     return NULL;
    // }

    unsigned long k;
    if (temp == root_map)
    {
        k = random_val() % ss_count;
        ssn = &server_registry[k];
    }
    else if (temp->ss_id >=0 && server_registry[temp->ss_id].down == 0)
    {
        ssn = &server_registry[temp->ss_id];
        k = temp->ss_id;
    }
    // else if (temp->ss_bk1 && server_registry[temp->ss_bk1].down == 0)
    // {
    //     ssn = &server_registry[temp->ss_bk1];
    //     k = temp->ss_bk1;
    // }
    // else if (temp->ss_bk2 && server_registry[temp->ss_bk2].down == 0)
    // {
    //     ssn = &server_registry[temp->ss_bk2];
    //     k = temp->ss_bk2;
    // }
    if(ssn->down == 1)
    {
        msg.type = UNAVAILABLE;
        logns(FAILURE, "Returning create directory from %s:%d, due to %s being unavailable", ip, port, msg.data);
        send_txn( sock, &msg, sizeof(msg), 0, r, 0);
        reqfree(r);
        return NULL;
    }

    // perror("py");

    msg.type = CREATE_D;
    sprintf(ss_ip, "%s", ssn->ip);
    ss_port = ssn->name_port;
    logns(PROGRESS, "Redirecting create directory request from %s:%d, to %s:%d", ip, port, ss_ip, ss_port);

    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ss_port);
    addr.sin_addr.s_addr = inet_addr(ss_ip);

    ss_sock = socket_txn(AF_INET, SOCK_STREAM, 0, r, 1);
    r->newsock = ss_sock;
    connect(ss_sock, (struct sockaddr *)&addr, sizeof(addr));
    booking(&msg);
    send_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);
    recv_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);

    int bytes;
    metadata_t *info;

    switch (msg.type)
    {
    case CREATE_D + 1:
        bytes = atoi(msg.data);
        logns(COMPLETION, "Received create directory acknowledgment from %s:%d", ss_ip, ss_port);
        break;
    default:
        booking(&msg);
        send_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);
        reqfree(r);
        return NULL;
    }

    info = (metadata_t *)malloc(bytes);
    r->allocptr = (void *)info;
    void *ptr = info;
    void *end = ptr + bytes;

    while (1)
    {
        recv_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);
        switch (msg.type)
        {
        case STOP:
            logns(COMPLETION, "Received %d bytes of metadata from %s:%d", bytes, ss_ip, ss_port);
            d_triesnode *dtnode;
            dtnode = insert_directory(temp->path, msg.file_name, temp->ss_id);
            put_directory((d_node**)temp->directories, msg.file_name, dtnode, temp->ss_id);
            msg.type = CREATE_D + 1;
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, r, 1);
            reqfree(r);
            return NULL;

        case CREATE_D + 1:
            if (end >= ptr + SIZE)
            {
                memcpy(ptr, msg.data, SIZE);
                ptr += SIZE;
                continue;
            }
            else if (end > ptr)
            {
                memcpy(ptr, msg.data, (void *)end - ptr);
                ptr = end;
                continue;
            }

        default:
            msg.type = UNAVAILABLE;
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, r, 1);
            reqfree(r);
            return NULL;
        }
    }
    close(ss_sock);
    booking(&msg);
    send_txn(sock, &msg, sizeof(msg), 0, r, 1);
    reqfree(r);
    return NULL;
}

void *ns_create_f(void *arg)
{
    request_t *r = arg;
    message_t msg = r->msg;
    int sock = r->sock;

    char ip[INET_ADDRSTRLEN];
    int port = ntohs(r->addr.sin_port);
    const char *ret = inet_ntop(AF_INET, &(r->addr.sin_addr), ip, INET_ADDRSTRLEN);
    if (ret == NULL)
    {
        perror_tpx(r, "inet_ntop");
    }
    logns(EVENT, "Received create file request from %s:%d, for %s", ip, port, msg.path);

    d_node *temp;
    f_node *f = get(&cache, msg.path);
    if (strcmp(msg.path, "home") == 0)
    {
        temp = root_map;
    }
    else if (f == NULL)
    {
        temp = is_valid_path(root_map, msg.path);
        if (temp == NULL)
        {
        printf("%s\n", msg.path);
            logns(FAILURE, "Returning create directory request from %s:%d, due to invalid path", ip, port);
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, r, 1);
            reqfree(r);
            return NULL;
        }
        f = is_file(temp->files, msg.file_name);
        // if(f != NULL)
        //     put(&cache,f->name->name,(void*)f);
    }

    storage_t *ssn;
    char ss_ip[INET_ADDRSTRLEN];
    int ss_sock;
    int ss_port;

    if (f != NULL)
    {
        msg.type = EXISTS;
        logns(FAILURE, "Returning create file request from %s:%d, due to collision", ip, port);
        booking(&msg);
        send_txn(sock, &msg, sizeof(msg), 0, r, 1);
        reqfree(r);
        return NULL;
    }

    // char parent[PATH_MAX];
    // get_parent_dir(parent, msg.data);

    // f = get(&cache, msg.path);

    // if (f == NULL)
    // {
    //     temp = is_valid_path(root_map, msg.path);
    //     f = is_file(temp->files, msg.file_name);
    //     if(f != NULL)
    //         put(&cache,f->name->name,(void*)f);
    // }

    // if (f == NULL)
    // {
    //     msg.type = NOTFOUND;
    //     logns(FAILURE, "Returning create file request from %s:%d, for unknown parent directory %s", ip, port, parent);
    //     send_txn(sock, &msg, sizeof(message_t), 0, r, 1);
    //     reqfree(r);
    //     return NULL;
    // }
    unsigned long k;
    if (temp == root_map)
    {
        k = random_val() % ss_count;
        ssn = &server_registry[k];
    }
    else if (temp->ss_id >= 0 && server_registry[temp->ss_id].down == 0)
    {
        ssn = &server_registry[temp->ss_id];
        k = temp->ss_id;
    }
    // else if (temp->ss_bk1 >= 0 && server_registry[temp->ss_bk1].down == 0)
    // {
    //     ssn = &server_registry[temp->ss_bk1];
    //     k = temp->ss_bk1;
    // }
    // else if (temp->ss_bk2 >= 0 && server_registry[temp->ss_bk2].down == 0)
    // {
    //     ssn = &server_registry[temp->ss_bk2];
    //     k = temp->ss_bk2;
    // }
    if(ssn->down == 1)
    {
        msg.type = UNAVAILABLE;
        logns(FAILURE, "Returning create file from %s:%d, due to %s being unavailable", ip, port, msg.data);
        send_txn(sock, &msg, sizeof(msg), 0, r, 1);
        reqfree(r);
        return NULL;
    }

    msg.type = CREATE_F;
    sprintf(ss_ip, "%s", ssn->ip);
    ss_port = ssn->name_port;
    logns(PROGRESS, "Redirecting create file request from %s:%d, to %s:%d", ip, port, ss_ip, ss_port);
    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ss_port);
    addr.sin_addr.s_addr = inet_addr(ss_ip);
    printf("%s %d\n", ip, port);

    ss_sock = socket_txn(AF_INET, SOCK_STREAM, 0, r, 1);
    r->newsock = ss_sock;
    if (connect(ss_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        perror("connect error");
    booking(&msg);
    send_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);
    recv_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);

    int bytes;
    metadata_t *info;
    switch (msg.type)
    {
    case CREATE_F + 1:
        bytes = atoi(msg.data);
        logns(COMPLETION, "Received create directory acknowledgment from %s:%d", ss_ip, ss_port);
        break;
    default:
        booking(&msg);
        send_txn(sock, &msg, sizeof(msg), 0, r, 0);
        reqfree(r);
        return NULL;
    }

    info = (metadata_t *)malloc(bytes);
    r->allocptr = (void *)info;
    void *ptr = info;
    void *end = ptr + bytes;

    while (1)
    {
        recv_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);
        switch (msg.type)
        {
        case STOP:
            logns(COMPLETION, "Received %d bytes of metadata from %s:%d", bytes, ss_ip, ss_port);
            printf("%s\n", msg.file_name);
            insert_file(temp->path, msg.file_name, k);
            put_file(temp, msg.file_name, k);
            msg.type = CREATE_D + 1;
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, r, 1);
            reqfree(r);
            return NULL;

        case CREATE_F + 1:
            if (end >= ptr + SIZE)
            {
                memcpy(ptr, msg.data, SIZE);
                ptr += SIZE;
                break;
            }
            else if (end > ptr)
            {
                memcpy(ptr, msg.data, (void *)end - ptr);
                ptr = end;
                break;
            }

        default:
            msg.type = UNAVAILABLE;
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, r, 1);
            reqfree(r);
            return NULL;
        }
    }
    close(ss_sock);

    booking(&msg);
    send_txn(sock, &msg, sizeof(msg), 0, r, 1);
    reqfree(r);
    return NULL;
}

void *ns_delete_f(void *arg) 
{
    request_t *r = arg;
    message_t msg = r->msg;
    int sock = r->sock;

    char ip[INET_ADDRSTRLEN];
    int port = ntohs(r->addr.sin_port);
    const char *ret = inet_ntop(AF_INET, &(r->addr.sin_addr), ip, INET_ADDRSTRLEN);
    if (ret == NULL) {
        perror_tpx(r, "inet_ntop");
    }
    logns(EVENT, "Received delete file request from %s:%d, for %s", ip, port, msg.path);

    d_node *directory;
    f_node *file = get(&cache, msg.path);
    if (strcmp(msg.path, "home") == 0) {
        directory = root_map;
    } else if (file == NULL) {
        directory = is_valid_path(root_map, msg.path);
        if (directory == NULL) {
            logns(FAILURE, "Returning delete file request from %s:%d, due to invalid path", ip, port);
            msg.type = INVALID;
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, r, 1);
            reqfree(r);
            return NULL;
        }
        file = is_file(directory->files, msg.file_name);
    }

    if (file == NULL) {
        msg.type = NOTFOUND;
        logns(FAILURE, "Returning delete file request from %s:%d, file not found", ip, port);
        booking(&msg);
        send_txn(sock, &msg, sizeof(msg), 0, r, 1);
        reqfree(r);
        return NULL;
    }

    storage_t *ssn;
    char ss_ip[INET_ADDRSTRLEN];
    int ss_sock;
    int ss_port;
    int ss_id;

    if (file->ss_id >= 0 && server_registry[file->ss_id].down == 0) {
        ssn = &server_registry[file->ss_id];
        ss_id = file->ss_id;
    } 
    //else if (file->ss_bk1 >= 0 && server_registry[file->ss_bk1].down == 0) {
    //     ssn = &server_registry[file->ss_bk1];
    //     ss_id = file->ss_bk1;
    // } else if (file->ss_bk2 >= 0 && server_registry[file->ss_bk2].down == 0) {
    //     ssn = &server_registry[file->ss_bk2];
    //     ss_id = file->ss_bk2;
    // } 
    if(ssn->down == 1)
    {
        msg.type = UNAVAILABLE;
        logns(FAILURE, "Returning delete file request from %s:%d, no available storage server", ip, port);
        booking(&msg);
        send_txn(sock, &msg, sizeof(msg), 0, r, 1);
        reqfree(r);
        return NULL;
    }

    sprintf(ss_ip, "%s", ssn->ip);
    ss_port = ssn->name_port;
    logns(PROGRESS, "Redirecting delete file request from %s:%d, to %s:%d", ip, port, ss_ip, ss_port);

    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ss_port);
    addr.sin_addr.s_addr = inet_addr(ss_ip);

    ss_sock = socket_txn(AF_INET, SOCK_STREAM, 0, r, 1);
    r->newsock = ss_sock;
    if (connect(ss_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        msg.type = UNAVAILABLE;
        logns(FAILURE, "Failed to connect to storage server %s:%d", ss_ip, ss_port);
        booking(&msg);
        send_txn(sock, &msg, sizeof(msg), 0, r, 1);
        reqfree(r);
        return NULL;
    }

    msg.type = DELETE_F;
    booking(&msg);
    send_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);
    recv_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);

    // if (msg.type != DELETE_F + 1) {
    //     send_txn(sock, &msg, sizeof(msg), 0, r, 1);
    //     close(ss_sock);
    //     reqfree(r);
    //     return NULL;
    // }

    delete_file(directory->path, msg.file_name);
    remove_file(directory, msg.file_name);

    // if (!trie_success || !hash_success) {
    //     msg.type = ERROR;
    //     logns(FAILURE, "Failed to delete file from data structures");
    //     send_txn(sock, &msg, sizeof(msg), 0, r, 1);
    //     close(ss_sock);
    //     reqfree(r);
    //     return NULL;
    // }

    msg.type = DELETE_F + 1;
    logns(COMPLETION, "Successfully deleted file %s", msg.file_name);
    booking(&msg);
    send_txn(sock, &msg, sizeof(msg), 0, r, 1);
    
    close(ss_sock);
    reqfree(r);
    return NULL;
}

void *ns_delete_d(void *arg)
{
    request_t *r = arg;
    message_t msg = r->msg;
    int sock = r->sock;

    char ip[INET_ADDRSTRLEN];
    int port = ntohs(r->addr.sin_port);
    const char *ret = inet_ntop(AF_INET, &(r->addr.sin_addr), ip, INET_ADDRSTRLEN);
    if (ret == NULL)
    {
        perror_tpx(r, "inet_ntop");
    }
    logns(EVENT, "Received delete directory request from %s:%d, for %s", ip, port, msg.path);

    char temppath[PATH_MAX];
    strcat(temppath, msg.path);
    strcat(temppath, "/");
    strcat(temppath, msg.file_name);

    printf("temppath: %s", temppath);

    d_node *d = get(&cache, msg.path);

    if (strcmp(msg.path, "home") == 0 || strcmp(msg.path, "/") == 0)
    {
        d = is_valid_path(root_map, msg.file_name);
    }
    else if (d == NULL)
    {
        d = is_valid_path(root_map, msg.path);
        if (d == NULL)
        {
            logns(FAILURE, "Returning delete directory request from %s:%d, due to invalid path", ip, port);
            msg.type = INVALID;
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, r, 1);
            reqfree(r);
            return NULL;
        }
        d = is_valid_path(root_map, temppath);
        if (d == NULL)
        {
            logns(NOTFOUND, "Returning delete directory request from %s:%d due to directory not found", ip, port);
            msg.type = INVALID;
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, r, 1);
            reqfree(r);
            return NULL;
        }
    }

    storage_t *ssn = (storage_t*)malloc(sizeof(storage_t));
    char ss_ip[INET_ADDRSTRLEN];
    int ss_sock;
    int ss_port;
    int ss_id;

    d_node* temp = is_valid_path(root_map, msg.path);

    if (temp->ss_id >= 0 && server_registry[temp->ss_id].down == 0)
    {
        ssn = &server_registry[temp->ss_id];
        ss_id = temp->ss_id;
    }
    // else if (temp->ss_bk1 >= 0 && server_registry[temp->ss_bk1].down == 0)
    // {
    //     ssn = &server_registry[temp->ss_bk1];
    //     ss_id = temp->ss_bk1;
    // }
    // else if (temp->ss_bk2 >= 0 && server_registry[temp->ss_bk2].down == 0)
    // {
    //     ssn = &server_registry[temp->ss_bk2];
    //     ss_id = temp->ss_bk2;
    // }
    if(ssn->down == 1) 
    {
        msg.type = UNAVAILABLE;
        logns(FAILURE, "Returning delete file request from %s:%d, no available storage server", ip, port);
        booking(&msg);
        send_txn(sock, &msg, sizeof(msg), 0, r, 1);
        reqfree(r);
        return NULL;
    }

    sprintf(ss_ip, "%s", ssn->ip);
    ss_port = ssn->name_port;
    logns(PROGRESS, "Redirecting delete directory request from %s:%d, to %s:%d", ip, port, ss_ip, ss_port);

    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ss_port);
    addr.sin_addr.s_addr = inet_addr(ss_ip);

    ss_sock = socket_txn(AF_INET, SOCK_STREAM, 0, r, 1);
    r->newsock = ss_sock;
    
    if (connect(ss_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        logns(FAILURE, "Failed to connect to storage server %s:%d", ss_ip, ss_port);
        msg.type = UNAVAILABLE;
        booking(&msg);
        send_txn(sock, &msg, sizeof(msg), 0, r, 1);
        reqfree(r);
        return NULL;
    }

    msg.type = DELETE_D;
    booking(&msg);
    send_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);
    recv_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);

    d_node *parent = is_valid_path(root_map, msg.path);
    if (parent != NULL)
    {
        remove_directory_recursive(parent->directories, msg.file_name);
        delete_directory(root_trie, msg.path);
    }

    // if (msg.type != DELETE_D + 1)
    // {
    //     logns(FAILURE, "Storage server failed to delete directory");
    //     send_txn(sock, &msg, sizeof(msg), 0, r, 1);
    //     close(ss_sock);
    //     reqfree(r);
    //     return NULL;
    // }

    msg.type = DELETE_D + 1;
    logns(COMPLETION, "Successfully deleted directory %s", msg.path);
    booking(&msg);
    send_txn(sock, &msg, sizeof(msg), 0, r, 1);

    close(ss_sock);
    reqfree(r);
    return NULL;
}


void *ns_read(void *arg)
{
    request_t *r = arg;
    message_t msg = r->msg;
    int sock = r->sock;

    char *ip = (char *)malloc(INET_ADDRSTRLEN * sizeof(char));
    int port = ntohs(r->addr.sin_port);
    const char *ret = inet_ntop(AF_INET, &(r->addr.sin_addr), ip, INET_ADDRSTRLEN);
    if (ret == NULL)
    {
        perror_tpx(r, "inet_ntop");
    }

    logns(EVENT, "Received read request from %s:%d, for %s", ip, port, msg.path);
    f_node *f = get(&cache, msg.path);

    if (f == NULL)
    {
        d_node *temp = is_valid_path(root_map, msg.path);
        f = is_file(temp->files, msg.file_name);
        if (f != NULL)
            put(&cache, f->name->name, (void *)f);
    }

    if (f == NULL)
    {
        msg.type = NOTFOUND;
        logns(FAILURE, "Returning read request from %s:%d, for unknown file %s", ip, port, msg.data);
    }
    else
    {
        printf("%s\n", f->name->name);
        int ssn;
        if (f->ss_id >= 0 && server_registry[f->ss_id].down == 0)
        {
            ssn = f->ss_id;
            strcpy(msg.data,"Normal");
        }
        else if (f->ss_bk1 >= 0 && server_registry[f->ss_bk1].down == 0)
        {
            ssn = f->ss_bk1;
            strcpy(msg.data, "Backup");
        }
        else if (f->ss_bk2 >= 0 && server_registry[f->ss_bk2].down == 0)
        {
            ssn = f->ss_bk2;
            strcpy(msg.data,"Backup");
        }
        else
        {
            msg.type = UNAVAILABLE;
            logns(FAILURE, "Returning read file request from %s:%d, no available storage server", ip, port);
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, r, 1);
            reqfree(r);
            return NULL;
        }
        
        msg.type = READ + 1;
        // bzero(msg.data, BUFSIZE);
        sprintf(msg.send_ip.ip, "%s", server_registry[ssn].ip);
        msg.send_ip.port = server_registry[ssn].client_port;
        logns(COMPLETION, "Redirecting read request from %s:%d, to %s:%d", ip, port, server_registry[ssn].ip, server_registry[ssn].client_port);
    }
    booking(&msg);
    send_txn(sock, &msg, sizeof(msg), 0, r, 1);

    reqfree(r);
    return NULL;
}

void *ns_write(void *arg)
{
    request_t *r = arg;
    message_t msg = r->msg;
    int sock = r->sock;

    char *ip = (char *)malloc(INET_ADDRSTRLEN * sizeof(char));
    int port = ntohs(r->addr.sin_port);
    const char *ret = inet_ntop(AF_INET, &(r->addr.sin_addr), ip, INET_ADDRSTRLEN);
    if (ret == NULL)
    {
        perror_tpx(r, "inet_ntop");
    }

    logns(EVENT, "Received write request from %s:%d, for %s", ip, port, msg.path);
    f_node *f = get(&cache, msg.path);
    printf("%s\n", msg.path);
    if (f == NULL)
    {
        d_node *temp = is_valid_path(root_map, msg.path);
        if(temp == NULL){
            perror("temp is null");
            return;
        }
        f = is_file(temp->files, msg.file_name);
        if (f != NULL)
            put(&cache, f->name->name, arg);
    }

    if (f == NULL)
    {
        msg.type = NOTFOUND;
        logns(FAILURE, "Returning write request from %s:%d, for unknown file %s", ip, port, msg.data);
    }
    else
    {
        msg.type = WRITE + 1;
        int ssn;
        if (f->ss_id >= 0 && server_registry[f->ss_id].down == 0)
        {
            ssn = f->ss_id;
        }
        // else if (f->ss_bk1 && server_registry[f->ss_bk1].down == 0)
        // {
        //     ssn = f->ss_bk1;
        // }
        // else if (f->ss_bk2 && server_registry[f->ss_bk2].down == 0)
        // {
        //     ssn = f->ss_bk2;
        // }
        else
        {
            msg.type = UNAVAILABLE;
            logns(FAILURE, "Returning write file request from %s:%d, no available storage server", ip, port);
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, r, 1);
            reqfree(r);
            return NULL;
        }
        memset(msg.data, 0, SIZE);
        sprintf(msg.send_ip.ip, "%s", server_registry[ssn].ip);
        // sprintf(msg.send_ip.port, "%d", server_registry[ssn].client_port);
        msg.send_ip.port = server_registry[ssn].client_port;
        logns(COMPLETION, "Redirecting read request from %s:%d, to %s:%d", ip, port, server_registry[ssn].ip, server_registry[ssn].client_port);
    }
    booking(&msg);
    send_txn(sock, &msg, sizeof(msg), 0, r, 1);

    reqfree(r);

    return NULL;
}

void *ns_list(void *arg)
{
    request_t *req = arg;
    message_t msg = req->msg;
    int sock = req->sock;

    char ip[INET_ADDRSTRLEN];
    int port = ntohs(req->addr.sin_port);
    inet_ntop(AF_INET, &(req->addr.sin_addr), ip, INET_ADDRSTRLEN);
    logns(EVENT, "Received list request from %s:%d", ip, port);

    // Start sending directory and file information
    msg.type = LIST + 1;
    logns(PROGRESS, "Starting to send directory and file data...");
    send_list_files(req, root_map);

    // Send STOP message to indicate completion
    msg.type = STOP;
    bzero(msg.data, SIZE);
    booking(&msg);
    send_txn(sock, &msg, sizeof(msg), 0, req, 1);
    logns(COMPLETION, "Completed sending directory and file data to %s:%d", ip, port);

    reqfree(req);
    return NULL;
}

void *ns_give_info(void *arg)
{
    request_t *r = arg;
    message_t msg = r->msg;
    int sock = r->sock;

    char *ip = (char *)malloc(INET_ADDRSTRLEN * sizeof(char));
    int port = ntohs(r->addr.sin_port);
    const char *ret = inet_ntop(AF_INET, &(r->addr.sin_addr), ip, INET_ADDRSTRLEN);
    if (ret == NULL)
    {
        perror_tpx(r, "inet_ntop");
    }

    logns(EVENT, "Received info request from %s:%d, for %s", ip, port, msg.path);
    f_node *f = get(&cache, msg.path);

    if (f == NULL)
    {
        d_node *temp = is_valid_path(root_map, msg.path);
        f = is_file(temp->files, msg.file_name);
        if (f != NULL)
            put(&cache, f->name->name, (void *)f);
    }

    if (f == NULL)
    {
        msg.type = NOTFOUND;
        logns(FAILURE, "Returning info request from %s:%d, for unknown file %s", ip, port, msg.data);
    }
    else
    {
        printf("%s\n", f->name->name);
        int ssn;
        if (f->ss_id >= 0 && server_registry[f->ss_id].down == 0)
        {
            ssn = f->ss_id;
        }
        else if (f->ss_bk1 >= 0 && server_registry[f->ss_bk1].down == 0)
        {
            ssn = f->ss_bk1;
        }
        else if (f->ss_bk2 >= 0 && server_registry[f->ss_bk2].down == 0)
        {
            ssn = f->ss_bk2;
        }
        else
        { 
            msg.type = UNAVAILABLE;
            logns(FAILURE, "Returning info file request from %s:%d, no available storage server", ip, port);
            booking(&msg);
            send_txn(sock, &msg, sizeof(msg), 0, r, 1);
            reqfree(r);
            return NULL;
        }
        msg.type = INFO + 1;
        bzero(msg.data, BUFSIZE);
        sprintf(msg.send_ip.ip, "%s", server_registry[ssn].ip);
        msg.send_ip.port = server_registry[ssn].client_port;
        booking(&msg);
        send_txn(sock, &msg, sizeof(msg), 0, r, 1);
        logns(COMPLETION, "Redirecting info file request from %s:%d, to %s:%d", ip, port, server_registry[ssn].ip, server_registry[ssn].client_port);
    }
    // msg.type = STOP;
    // booking(&msg);
    // send_txn(sock, &msg, sizeof(msg), 0, r, 1);

    reqfree(r);
    return NULL;
}

void *ns_copy_internal(void *arg)
{
    return NULL;
}

void *ns_copy_across(void *arg)
{
    return NULL;
}

void *ns_backup(void *arg)
{
    return NULL;
}

void *ns_update(void *arg)
{
    return NULL;
}

void *ns_join(void *arg)
{
    // storage server joining
    request_t *req = arg;
    pthread_mutex_lock(&registry_lock);

    server_registry = realloc(server_registry, sizeof(storage_t) * (ss_count + 1));
    if (!server_registry)
    {
        perror("realloc");
        pthread_mutex_unlock(&registry_lock);
        return NULL;
    }

    strcpy(server_registry[ss_count].ip, req->msg.recv_ip.ip);
    server_registry[ss_count].name_port = req->msg.recv_ip.port;
    server_registry[ss_count].client_port = req->msg.recv_ip.port - 1;
    server_registry[ss_count].storage_port = req->msg.recv_ip.port - 2;
    strcpy(server_registry[ss_count].path, req->msg.path);  
    server_registry[ss_count].down = 0;
    ss_count++;

    pthread_mutex_unlock(&registry_lock);

    logns(EVENT, "Registered Storage Server: %s:%d", req->msg.recv_ip.ip, req->msg.recv_ip.port);

    message_t response =
        {
            .type = JOIN + 1,
        };

    snprintf(response.data, SIZE, "Registered Storage Server");
    send(req->sock, &response, sizeof(response), 0);
    return NULL;
}


int ping_ss(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 0;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    struct timeval timeout;
    timeout.tv_sec = PING_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    int result = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    close(sockfd);

    if (result < 0) {
        if (errno == ETIMEDOUT || errno == ECONNREFUSED) {
            return 0;  // Connection failed or timed out
        }
    }
    return 1;  // Server responded
}


void* ns_ping(void* arg) {
    while(1)
    {for (int i = 0; i < ss_count; i++) {
        if (server_registry[i].ip[0] == '\0') {
            continue;  
        }

        int attempts = 0;
        int responded = 0;

        while (attempts < PING_ATTEMPTS) {
            if (ping_ss(server_registry[i].ip, server_registry[i].storage_port)) {
                responded = 1;
                break;  // Server responded, no need for further attempts
            }
            attempts++;
        }

        if (!responded) {
            server_registry[i].down = 1;  // Mark server as down
            logns(FAILURE,"Returning ping request from %s:%d, server is down", server_registry[i].ip, server_registry[i].storage_port);
        } else {
            server_registry[i].down = 0;  // Mark server as up
            logns(PROGRESS, "Server %s:%d is up", server_registry[i].ip, server_registry[i].storage_port);
        }
    }
    sleep(PING_TIMEOUT);
    }
    return NULL;
}

void* assign_backup(void* arg)
{
    for(int i = 0; i <ss_count;i++)
    {
       server_registry[i].ss_bkp1 = i%3 +1;
       server_registry[i].ss_bkp2 = i%3 +2;
    }
}
void* handle_backup(void* arg)
{
    while(1)
    {
        pthread_t assign;
        if(ss_count <= 2)
        {
            continue;
        }
        else if(ss_count > 2){
            pthread_create_tx(&assign, NULL, assign_backup, server_registry);
        }
        else{
            sleep(5);
        }
    }
    return NULL;
}


void *ns_stop(void *arg)
{
    return NULL;
}

void *ns_stream(void *arg)
{
    request_t *r = arg;
    message_t msg = r->msg;
    int sock = r->sock;

    char *ip = (char *)malloc(INET_ADDRSTRLEN * sizeof(char));
    int port = ntohs(r->addr.sin_port);
    const char *ret = inet_ntop(AF_INET, &(r->addr.sin_addr), ip, INET_ADDRSTRLEN);
    if (ret == NULL)
    {
        perror_tpx(r, "inet_ntop");
    }

    logns(EVENT, "Received stream request from %s:%d, for %s", ip, port, msg.path);
    f_node *f = get(&cache, msg.path);


    if (f == NULL)
    {
        d_node *temp = is_valid_path(root_map, msg.path);
        f = is_file(temp->files, msg.file_name);
        if (f != NULL)
            put(&cache, f->name->name, (void *)f);
    }
    if(f)
    {
        // printf("%s\n", f->name->name);
        int ssn;
        if (f->ss_id >= 0&& server_registry[f->ss_id].down == 0)
        {
            ssn = f->ss_id;
        }
        else if (f->ss_bk1 >= 0 && server_registry[f->ss_bk1].down == 0)
        {
            ssn = f->ss_bk1;
        }
        else if (f->ss_bk2 >= 0 && server_registry[f->ss_bk2].down == 0)
        {
            ssn = f->ss_bk2;
        }
        msg.type = STREAM + 1;
        bzero(msg.data, BUFSIZE);
        sprintf(msg.send_ip.ip, "%s", server_registry[ssn].ip);
        msg.send_ip.port = server_registry[ssn].client_port;
        logns(COMPLETION, "Redirecting stream request from %s:%d, to %s:%d", ip, port, server_registry[ssn].ip, server_registry[ssn].client_port);
    }
    else{
        msg.type = NOTFOUND;
    }    booking(&msg);
    send_txn(sock, &msg, sizeof(msg), 0, r, 1);

    reqfree(r);
    return NULL;
}

void *ns_handle_listen(void *arg)
{
    return NULL;
}

void *ns_handle_error(void *arg)
{
    return NULL;
}
