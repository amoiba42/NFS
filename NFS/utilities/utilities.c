#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include "utilities.h"
#include "../client/client.h"

extern logfile_t* logfile;
extern storage_t* server_registry;
extern int ss_count;
extern pthread_mutex_t registry_lock;
char* NAME_IP = NULL;

void logevent(enum caller c, enum level lvl, const char* message, ...)
{
  pthread_mutex_lock_tx(&(logfile->lock));
  va_list args;
  va_start(args, message);

  if (lvl == STATUS)
  {
      timestamp(stdout);
      fprintf(stdout, PINK_NORMAL);
      vfprintf(stdout, message, args);
      fprintf(stdout, RESET "\n");
  }
  else if (lvl == EVENT)
  {
      timestamp(stdout);
      fprintf(stdout, CYAN_NORMAL);
      vfprintf(stdout, message, args);
      fprintf(stdout, RESET "\n");
  }
  else if (lvl == PROGRESS)
  {
#ifdef DEBUG
      timestamp(stdout);
      fprintf(stdout, MAGENTA_NORMAL);
      vfprintf(stdout, message, args);
      fprintf(stdout, RESET "\n");
#endif
  }
  else if (lvl == COMPLETION)
  {
      timestamp(stdout);
      fprintf(stdout, GREEN_NORMAL);
      vfprintf(stdout, message, args);
      fprintf(stdout, RESET "\n");
  }
  else if (lvl == FAILURE)
  {
      timestamp(stderr);
      fprintf(stderr, RED_NORMAL);
      vfprintf(stderr, message, args);
      fprintf(stderr, RESET "\n");
  }

#ifdef LOG
  va_start(args, message);
  if (logfile->path[0] != 0) {
    FILE* outfile = fopen_tx(logfile->path, "a+");
    vfprintf(outfile, message, args);
    fclose(outfile);
  }
#endif

  va_end(args);
  pthread_mutex_unlock_tx(&(logfile->lock));
}

request_t* alloc(void)
{
  request_t* r = (request_t*)malloc(sizeof(request_t));
  r->addrlen = sizeof(r->addr);
  r->sock = -5000;
  r->newsock = -5000;
  return r;

}

void reqfree(request_t* req)
{
  if (req->sock != -1)
    close(req->sock);
  if (req->newsock != -1)
    close(req->newsock);
  if (req->allocptr != NULL)
    free(req->allocptr);
  free(req);
}

void timestamp(FILE* stream)
{
  time_t timer;
  char buffer[24];
  struct tm* tm_info;

  timer = time(NULL);
  tm_info = localtime(&timer);

  strftime(buffer, 24, "%Y-%m-%d %H:%M:%S", tm_info);
  fprintf(stream, "\033[90m" "%s " "\033[0m", buffer);
}

char* get_local_ip() 
{
    struct ifaddrs *interfaces, *ifa;
    char *ip = NULL;

    // Get the list of interfaces
    if (getifaddrs(&interfaces) == 0) 
    {
        // Iterate through interfaces to find the first non-loopback IPv4 address
        for (ifa = interfaces; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in* sockaddr = (struct sockaddr_in*) ifa->ifa_addr;
                if (strcmp(ifa->ifa_name, "lo") != 0) 
                {
                    ip = inet_ntoa(sockaddr->sin_addr);
                    break;
                }
            }
        }
        freeifaddrs(interfaces);
    }

    return ip ? ip : "127.0.0.1";  // Fallback to localhost if no valid IP found
}