#ifndef __HANDLERS_H
#define __HANDLERS_H

#include "../utilities/utilities.h"
#include "../utilities/api.h"
#include "trie.h"
#include "hash.h"
#include "lru.h"


#define PING_TIMEOUT 5   // Timeout for each ping in seconds
#define PING_ATTEMPTS 3 
// extern logfile_t *logfile;
// extern d_triesnode *root_trie;
// extern d_node *root_map;
// extern storage_t *server_registry;
// extern int ss_count;

extern LRUCache cache;

void* ns_create_d(void* arg);
void* ns_create_f(void* arg);
void* ns_delete_d(void* arg);
void* ns_delete_f(void* arg);
void* ns_read(void* arg);
void* ns_write(void* arg);
void* ns_list(void* arg);
void* ns_give_info(void* arg);
void* ns_copy(void* arg);
void* ns_copy_internal(void* arg);
void* ns_copy_across(void* arg);
void* ns_backup(void* arg);
void* ns_update(void* arg);
void* ns_join(void* arg);
void* ns_stop(void* arg);
void* ns_stream(void* arg);
void* ns_handle_listen(void* arg);
void* ns_handle_error(void *arg);
void* ns_ping(void* arg);
void* handle_backup(void* arg);

d_node* is_valid_path(d_node* root, const char* path);


#endif