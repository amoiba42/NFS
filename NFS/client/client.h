#ifndef __CLIENT_H
#define __CLIENT_H

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <strings.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "defs.h"
#include "../utilities/api.h"
#include "../utilities/utilities.h"


extern int name_socket;
#define logc(level, ...) logevent(CLIENT, level, __VA_ARGS__)


void request_read(int name_socket);
void request_write(int name_socket);
void request_create(int name_socket);
void request_delete(int name_socket);
void request_copy(int name_socket);
void request_list(int name_socket);
void request_stream(int name_socket);
void request_info(int name_socket);

#endif