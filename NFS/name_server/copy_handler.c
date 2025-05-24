#include "ns.h"

extern logfile_t *logfile;
extern d_triesnode *root_trie;
extern d_node *root_map;
extern storage_t *server_registry;
extern LRUCache cache;
extern int ss_count;
extern pthread_mutex_t registry_lock;

void *ns_copy(void *arg)
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

    char *current = (char *)malloc(PATH_MAX * sizeof(char));
    char *new = (char *)malloc(PATH_MAX * sizeof(char));

    sprintf(current, "%s", msg.path);
    recv_txn(sock, &msg, sizeof(message_t), 0, r, 1);
    sprintf(new, "%s", msg.path);

    d_node *current_node = get(&cache, current);
    if (current_node == NULL)
    {
        current_node = is_valid_path(root_map, current);
        if (current_node != NULL)
            put(&cache, current_node, r);
    }

    if (current_node == NULL)
    {
        msg.type = NOTFOUND;
        logns(FAILURE, "Returning copy request from %s:%d, for unknown file %s", ip, port, current);
        booking(&msg);
        send_txn(sock, &msg, sizeof(message_t), 0, r, 1);
        reqfree(r);
        return NULL;
    }

    char *parent = (char *)malloc(PATH_MAX * sizeof(char));

    d_node *new_node = get(&cache, parent);
    if (new_node == NULL)
    {
        new_node = is_valid_path(root_map, new);
        if (new_node != NULL)
            put(&cache, new_node, r);
    }

    if (new_node == NULL)
    {
        msg.type = NOTFOUND;
        logns(FAILURE, "Returning copy request from %s:%d, to unknown parent directory %s", ip, port, parent);
        booking(&msg);
        send_txn(sock, &msg, sizeof(message_t), 0, r, 1);
        reqfree(r);
        return NULL;
    }
    // available_server();

    // snode_t* sender = available_server(curnode);
    // snode_t* receiver = available_server(newnode);
    int receiver = -1, sender = -1;
        if (new_node->ss_id>=0 && server_registry[new_node->ss_id].down == 0)
        {
            receiver = new_node->ss_id;
        }
        else if (new_node->ss_bk1>=0 && server_registry[new_node->ss_bk1].down == 0)
        {
            receiver = new_node->ss_bk1;
        }
        else if (new_node->ss_bk2>=0 && server_registry[new_node->ss_bk2].down == 0)
        {
            receiver = new_node->ss_bk2;
        }
        if (current_node->ss_id>=0 && server_registry[current_node->ss_id].down == 0)
        {
            sender = current_node->ss_id;
        }
        else if (current_node->ss_bk1>=0 && server_registry[current_node->ss_bk1].down == 0)
        {
            sender = current_node->ss_bk1;
        }
        else if (current_node->ss_bk2>=0 && server_registry[current_node->ss_bk2].down == 0)
        {
            sender = current_node->ss_bk2;
        }

    if (sender == -1 || receiver == -1)
    {
        printf("%d %d\n", sender, receiver);
        msg.type = UNAVAILABLE;
        booking(&msg);
        send_txn(sock, &msg, sizeof(message_t), 0, r, 1);
        reqfree(r);
        return NULL;
    }

    char *ss_ip = (char *)malloc(INET_ADDRSTRLEN * sizeof(char));
    int ss_port;
    sprintf(ss_ip, "%s", server_registry[sender].ip);
    ss_port = server_registry[sender].name_port;

    logns(PROGRESS, "Redirecting create file request from %s:%d, to %s:%d", ip, port, ss_ip, ss_port);
    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ss_port);
    addr.sin_addr.s_addr = inet_addr(ss_ip);

    int ss_sock = socket(AF_INET, SOCK_STREAM, 0);
    r->newsock = ss_sock;
    if (connect(ss_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Can't connect");
        return -1;
    }

    if (sender == receiver)
    {
        perror("we");
        msg.type = COPY_INTERNAL;
        sprintf(msg.path, "%s", current);
        perror("we");
        booking(&msg);
        send_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);
        perror("we");
        sprintf(msg.path, "%s", new);
        booking(&msg);
        perror("we");
        send_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);
        logns(PROGRESS, "Forwared copy request to %s:%d, for %s", ss_ip, ss_port, current);

        perror("we");
        recv_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);
        if (msg.type == COPY_INTERNAL + 1)
        {
            logns(COMPLETION, "Received copy acknowledgment from %s:%d, for %s", ss_ip, ss_port, current);
            msg.type = COPY + 1;
        }
    }
    else
    {
        msg.type = COPY_ACROSS;
        sprintf(msg.path, "%s", current);
        booking(&msg);
        send_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);
        sprintf(msg.path, "%s", new);
        booking(&msg);
        send_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);
        sprintf(msg.recv_ip.ip, "%s", server_registry[receiver].ip);
        sprintf(msg.recv_ip.port, "%d", server_registry[receiver].storage_port);
        booking(&msg);
        send_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);
        logns(PROGRESS, "Forwarded copy request to %s:%d, for %s", ss_ip, ss_port, current);
        recv_txn(ss_sock, &msg, sizeof(msg), 0, r, 1);

        if (msg.type == COPY_ACROSS + 1)
        {
            logns(COMPLETION, "Received copy acknowledgment from %s:%d, for %s", ss_ip, ss_port, current);
            msg.type = COPY + 1;
        }
    }

    close(ss_sock);
    booking(&msg);
    send_txn(sock, &msg, sizeof(message_t), 0, r, 1);
    reqfree(r);

    return NULL;
}
