#ifndef __NS_H
#define __NS_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
// #include <pthread.h>
#include <arpa/inet.h>

#include "handlers.h"
#include "lru.h"
#include "trie.h"
#include "hash.h"

#include "../utilities/utilities.h"


#define logns(level, ...) logevent(SERVER, level, __VA_ARGS__)

int main(int argc, char* argv[]);
void* handler_thread(void* arg);
#endif