#include "ss.h"

extern logfile_t* logfile;

void* cl_listener_thread(void* arg)
{
    request_t* req = arg;
    recv(req->sock, &(req->msg), sizeof(req->msg), 0);

    if (req->msg.type == READ)
    {
        ss_handle_read(arg);
    }
    else if (req->msg.type == WRITE)
    {
        ss_handle_write(arg);
    }
    else if (req->msg.type == INFO)
    {
        ss_handle_info(arg);
    }
    else if(req->msg.type == STREAM)
    {
        ss_handle_audio(arg);
    }

    return NULL;
}

void* cl_listener(void* arg)
{
    listener_args_t* args = (listener_args_t*) arg;
    char* IP = args->ip;
    int port = args->port;

    request_t* req = (request_t*) calloc(1, sizeof(request_t));
    if (req == 0)
        perror("calloc");
    // request_t* req = arg;
    req->newsock = -1;
    req->addrlen = sizeof(req->addr);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    req->sock = sock;

    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(IP);
    

    bind(sock, (struct sockaddr*) &addr, sizeof(addr));
    listen(sock, 128);

    while (1)
    {
        req->allocptr = NULL;
        req->newsock = accept(sock, (struct sockaddr*) &(req->addr), &(req->addrlen));
        if(req->newsock < 0)
        {
            perror("Sock error");
        }
        char ip[INET_ADDRSTRLEN];
        int port = ntohs(req->addr.sin_port);
        inet_ntop(AF_INET, &(req->addr.sin_addr), ip, INET_ADDRSTRLEN);
        logst(EVENT, "Accepted connection from %s:%d", ip, port);

        request_t* newreq = (request_t*) calloc(1, sizeof(request_t));
        if (newreq == 0)
            perror("calloc");

        req->allocptr = (void*) newreq;
        newreq->sock = req->newsock;
        newreq->addr = req->addr;
        newreq->addrlen = req->addrlen;
        
        pthread_t worker;
        pthread_create(&worker, NULL, cl_listener_thread, newreq);
    }

    return NULL;
}

void* st_listener_thread(void* arg)
{
    request_t* req = arg;
    recv(req->sock, &(req->msg), sizeof(req->msg), 0);
    perror("ha;wa");
    if (req->msg.type == BACKUP)
    {
        
    }
    else if (req->msg.type == COPY_ACROSS)
    {
        ss_handle_copy_recv(req->sock, req);
    }
    else if (req->msg.type == UPDATE)
    {

    }

    return NULL;
}

void* st_listener(void* arg)
{
    listener_args_t* args = (listener_args_t*) arg;
    char* IP = args->ip;
    int port = args->port;

    request_t* req = (request_t*) calloc(1, sizeof(request_t));
    if (req == 0)
        perror("calloc");
    req->newsock = -1;
    req->addrlen = sizeof(req->addr);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    req->sock = sock;

    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(IP);

    bind(sock, (struct sockaddr*) &addr, sizeof(addr));
    listen(sock, 128);

    while (1)
    {
        req->allocptr = NULL;
        req->newsock = accept(sock, (struct sockaddr*) &(req->addr), &(req->addrlen));
        
        char ip[INET_ADDRSTRLEN];
        int port = ntohs(req->addr.sin_port);
        inet_ntop(AF_INET, &(req->addr.sin_addr), ip, INET_ADDRSTRLEN);
        logst(EVENT, "Accepted connection from %s:%d", ip, port);

        request_t* newreq = (request_t*) calloc(1, sizeof(request_t));
        if (newreq == 0)
            perror("calloc");
        newreq->newsock = -1;
        newreq->sock = req->newsock;
        newreq->addr = req->addr;
        newreq->addrlen = req->addrlen;

        req->allocptr = (void*) newreq;
        perror("32");
        pthread_t worker;
        pthread_create(&worker, NULL, st_listener_thread, newreq);
    }

    return NULL;
}

void* ns_listener_thread(void* arg)
{
    request_t* req = arg;
    recv(req->sock, &(req->msg), sizeof(req->msg), 0);

    if (req->msg.type == BACKUP)
    {

    }
    else if (req->msg.type == COPY_ACROSS)
    {
        ss_handle_copy_across(arg);  
    }
    else if (req->msg.type == COPY_INTERNAL)
    {
        ss_handle_copy_internal(arg);
    }
    else if (req->msg.type == CREATE_D)
    {
        ss_handle_create(arg, 1);
    }
    else if (req->msg.type == CREATE_F)
    {
        ss_handle_create(arg, 0);
    }
    else if (req->msg.type == DELETE_D)
    {
        ss_handle_delete(arg, 1);
    }
    else if (req->msg.type == DELETE_F)
    {
        ss_handle_delete(arg, 0);
    }
    else if (req->msg.type == PING)
    {
        ss_handle_ping(arg);
    }
    else if (req->msg.type == UPDATE)
    {

    }
    else    
    {

    }

    return NULL;
}

void* ns_listener(void* arg)
{
    listener_args_t* args = (listener_args_t*) arg;
    char* IP = args->ip;
    int port = args->port;
    printf("%s %d\n", IP, port);
    request_t* req = (request_t*) calloc(1, sizeof(request_t));
    if (req == 0)
        perror("calloc");
    req->newsock = -1;
    req->addrlen = sizeof(req->addr);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    req->sock = sock;

    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(IP);

    if(bind(sock, (struct sockaddr*) &addr, sizeof(addr))<0){
        perror("alpha");
        perror("bind");
    }
    listen(sock, 128);

    while (1)
    {
        req->allocptr = NULL;
        req->newsock = accept(sock, (struct sockaddr*) &(req->addr), &(req->addrlen));
        if(req->newsock < 0)perror("accept");
        
        char ip[INET_ADDRSTRLEN];
        int port = ntohs(req->addr.sin_port);
        inet_ntop(AF_INET, &(req->addr.sin_addr), ip, INET_ADDRSTRLEN);
        logst(EVENT, "Accepted connection from %s:%d", ip, port);

        request_t* newreq = (request_t*) calloc(1, sizeof(request_t));
        if (newreq == 0)
            perror("calloc");
        newreq->newsock = -1;
        newreq->sock = req->newsock;
        newreq->addr = req->addr;
        newreq->addrlen = req->addrlen;

        req->allocptr = (void*) newreq;

        pthread_t worker;
        pthread_create(&worker, NULL, ns_listener_thread, newreq);
    }

    return NULL;
}