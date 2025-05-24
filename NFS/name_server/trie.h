#ifndef TRIE_H
#define TRIE_H
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<stdint.h>
#include"../utilities/utilities.h"
#include"../utilities/api.h"

#define CREATE_FILE (void* (*)())init_f_trie
#define CREATE_DIRECTORY (void* (*)())init_d_trie
#define CHAR_SET 256
typedef struct d_triesnode d_triesnode;
typedef struct f_triesnode f_triesnode;
typedef struct file_info file_info;
typedef struct file_info directory_info;
struct d_triesnode{
    // bool end_of_word;
    directory_info* directory;
    d_triesnode* child[CHAR_SET];
    f_triesnode* files;
};
struct f_triesnode{
    // bool end_of_word;
    file_info* file;
    f_triesnode* child[CHAR_SET];
};
struct file_info{
    char path[PATH_MAX];
    char name[1024];
};

f_triesnode* init_f_trie();
d_triesnode* init_d_trie();
void insert_char(void* T,char c, void* (*init_func)());
d_triesnode* insert_directory(d_triesnode* T, char* str, int ss_id);
void insert_file(d_triesnode* T, char* str, int ss_id);
void find_prefix(d_triesnode* T, char *str);

void print_files(f_triesnode* T);
void print_directory(d_triesnode* T);

bool delete_file(d_triesnode* T, const char* filename);
bool delete_directory(d_triesnode* T, const char* dirname);

#endif