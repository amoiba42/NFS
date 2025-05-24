#ifndef __THREAD_H__
#define __THREAD_H__
#include <pthread.h>
#include "utilities.h"

void handle_thread_error(const char *msg);
int pthread_create_tx(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg);
int pthread_mutex_init_tx(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_lock_tx(pthread_mutex_t *mutex);
int pthread_mutex_unlock_tx(pthread_mutex_t *mutex);


#endif