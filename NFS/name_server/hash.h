#ifndef HASH_H
#define HASH_H
#include"trie.h"
#include"../utilities/utilities.h"

//yo nigga
#define MAX_FILES 4096
#define MAX_LENGTH 512
typedef struct d_node d_node;
typedef struct f_node f_node;
struct d_node
{
    directory_info* name;
    d_node *directories[MAX_FILES];
    f_node *files[MAX_FILES];
    d_triesnode* path;
    d_node *next;
    int ss_id;
    int ss_bk1;
    int ss_bk2;
};
struct f_node
{
    file_info* name;
    f_node *next;
    int ss_id;
    int ss_bk1;
    int ss_bk2;
};
int sumsqASCII( char *string);

void put_directory(d_node** d, char* str, d_triesnode* path, int ss_id);
void put_file(d_node* d, char* str, int ss_id);
void list_files(d_node*);
void hash_print_directory(d_node**, char*);
void hash_print_files(f_node**, char*);
void find_path(d_node*);
void send_list_files(request_t* req, d_node* root);
void send_hash_print_directory(request_t* req, d_node** root, char* path);
void send_hash_print_files(request_t* req, f_node** files, char* path);

bool remove_file(d_node* directory, const char* filename);
bool remove_directory_recursive(d_node** parent_directories, const char* dirname);

#endif