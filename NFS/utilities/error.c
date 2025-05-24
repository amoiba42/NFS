#include <errno.h>
#include <stdlib.h>
#include "utilities.h"

extern logfile_t* logfile;

void perror_tx(const char* s)
{
  pthread_mutex_lock_tx(&(logfile->lock));
  timestamp(stderr);
  fprintf(stderr, RED_NORMAL);
  perror(s);
  fprintf(stderr, RESET);
  fflush(stderr);
  pthread_mutex_unlock_tx(&(logfile->lock));
  exit(1);
}

void perror_tpx(request_t* req, const char* s)
{
  pthread_mutex_lock_tx(&(logfile->lock));
  timestamp(stderr);
  fprintf(stderr, RED_NORMAL);
  perror(s);
  fprintf(stderr, RESET);
  fflush(stderr);
  pthread_mutex_unlock_tx(&(logfile->lock));
  if(req)
  reqfree(req);
  pthread_exit(NULL);
}