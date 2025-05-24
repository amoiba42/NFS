#ifndef __SS_H
#define __SS_H

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "../utilities/api.h"
#include "../utilities/utilities.h"
#include "../utilities/thread.h"

#define logst(level, ...) logevent(STORAGE, level, __VA_ARGS__)

void* cl_listener(void* arg);
void* ns_listener(void* arg);
void* st_listener(void* arg);

void* ss_handle_write(void* arg);
void* ss_handle_read(void* arg);

// flag = 1 for directory, flag = 0 for file
void* ss_handle_create(void* arg, int flag);
void* ss_handle_delete(void* arg, int flag);

void* ss_handle_info(void* arg);

// flag = 1 for copy across, flag = 0 for copy internal
void* ss_handle_copy_across(void* arg);
void ss_handle_copy_recv(int client_socket, request_t *req);
void* ss_handle_copy_internal(void* arg);
void* ss_handle_ping(void* arg);
void* ss_handle_audio(void* arg);

typedef struct {
    char* ip;
    int port;
} listener_args_t;

#endif
