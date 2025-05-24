#ifndef _UTILITIES_H
#define _UTILITIES_H

#include "api.h"
#include "color.h"
#include "socket.h"
#include "thread.h"
#include <stdio.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>

enum caller {CLIENT, SERVER, STORAGE};
enum level {STATUS, EVENT, PROGRESS, COMPLETION, FAILURE};

void logevent(enum caller c, enum level lvl, const char* message, ...);

//error.c
void perror_tx(const char* s);
void perror_tpx(request_t* req, const char* s);

//socket.c
ssize_t send_txn(int sockfd, void *buf, size_t len, int flags, request_t *req, int use_tx);
ssize_t recv_txn(int sockfd, void *buf, size_t len, int flags, request_t *req, int use_tx);
int accept_txn(int sockfd, struct sockaddr *addr, socklen_t *addrlen, request_t *req, int use_tx);
int listen_txn(int sockfd, int backlog, request_t *req, int use_tx);
int socket_txn(int domain, int type, int protocol, request_t *req, int use_tx);
int setsocket_txn(int sockfd, int level, int optname, const void *optval, socklen_t optlen, request_t *req, int use_tx);
int bind_txn(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int connect_retry(int sockfd, const struct sockaddr* addr, socklen_t addrlen, int* timeout );
int connect_t(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
int connect_after(int sockfd, const struct sockaddr* addr, socklen_t addrlen, int timeout);


//utilities.c
void logevent(enum caller c, enum level lvl, const char* message, ...);
request_t* alloc(void);
void reqfree(request_t* req);
void timestamp(FILE* stream);

// extern logfile_t* logfile;
// extern storage_t* server_registry;
// extern int ss_count;

char* get_local_ip(void);

void booking(message_t* msg);

#endif