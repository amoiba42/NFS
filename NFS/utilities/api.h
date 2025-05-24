#ifndef __API_H
#define __API_H

#include <time.h>
#include <netdb.h>
#include <limits.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>

extern char* NAME_IP;

#define NAME_PORT 8080

#define SIZE 1024
#define PATH_MAX 4096

#define MAX_SS 10

#define READ 0                   // request codes
#define WRITE 2                  // +1 for response
#define CREATE_F 4
#define CREATE_D 6
#define LIST 10
#define INFO 12
#define COPY 14
#define COPY_INTERNAL 16
#define COPY_ACROSS 18
#define BACKUP 20
#define UPDATE 22
#define JOIN 24
#define PING 26
#define STOP 28
#define STREAM 30

#define DELETE_F 40
#define DELETE_D 42

#define INET_ADDRSTRLEN 16

#define INVALID -1
#define NOTFOUND -2
#define EXISTS -3
#define BEING_READ -4
#define RDONLY -5                // writes disabled on ss failure
#define XLOCK -6
#define PERM -7
#define UNAVAILABLE -8

#define BUFSIZE 4096             // assumption: greater than PATH_MAX, 4096
#define RETRY_DIFF 5             // seconds

#define COPY_COND 100            // set to indicate user defined operation
#define BACKUP_COND 101          // set to indicate naming server defined backup operation

typedef struct IPAddress IPAddress;
typedef struct __message message_t;
typedef struct __request request_t;
typedef struct __metadata metadata_t;
typedef struct __storage storage_t;
typedef struct __logfile logfile_t;


struct IPAddress{
    char ip[16];
    uint16_t port;
};

struct __message {
  int32_t type;
  char data[SIZE];
  IPAddress recv_ip;
  IPAddress send_ip;
  char path[PATH_MAX];
  char file_name[PATH_MAX];
};

 struct __request {
  int sock;
  int newsock;
  struct sockaddr_in addr;
  socklen_t addrlen;
  message_t msg;
  void* allocptr;
};

struct __metadata {
  char path[PATH_MAX];
  mode_t mode;
  off_t size;
  __time_t ctime;                // TODO: struct_stat.h may refer to struct timespec instead
  __time_t mtime;
};

struct __storage {
  char ip[INET_ADDRSTRLEN];
  char path[PATH_MAX];
  int32_t name_port;
  int32_t client_port;
  int32_t storage_port;
  int ss_id;
  int ss_bkp1;
  int ss_bkp2;
  int down;
};

struct __logfile {
  char path[SIZE];
  pthread_mutex_t lock;
};

// extern storage_t* server_registry;
// extern int ss_count;
// extern pthread_mutex_t registry_lock; 
#endif