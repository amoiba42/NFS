#include "trie.h"

/*
    This for searching based on user requests.
    This will search the string entered is a prefix or not. 
*/


void print_directory(d_triesnode* T) {
    if (T == NULL) return;

    // If current node has a file, print the file name and server info
    if (T->directory) {
        printf("Directory: %s\n", T->directory->name);
        print_files(T->files);
    }

    // Recurse for each child
    for (int i = 0; i < CHAR_SET; i++) {
        if (T->child[i]) {
            print_directory(T->child[i]);
        }
    }
}
void print_files(f_triesnode* T) {
    if (T == NULL) return;

    // If current node has a file, print the file name and server info
    if (T->file) {
        printf("Files: %s\n", T->file->name);
    }

    // Recurse for each child
    for (int i = 0; i < CHAR_SET; i++) {
        if (T->child[i]) {
            print_files(T->child[i]);
        }
    }
}


d_triesnode* init_d_trie(){
    d_triesnode* ret=(d_triesnode*)malloc(sizeof(d_triesnode));
    ret->directory = NULL;

    ret->files = NULL;
    for(int i = 0;i<CHAR_SET;i++)
        ret->child[i] = NULL;

    return ret;
}

f_triesnode* init_f_trie(){
    f_triesnode* ret=(f_triesnode*)malloc(sizeof(f_triesnode));
    ret->file = NULL;

    for(int i = 0;i<CHAR_SET;i++)
        ret->child[i] = NULL;

    return ret;
}

void insert_char(void* T,char c, void* (*init_func)()){
            
    if (init_func == CREATE_DIRECTORY) {
        // Handle as d_triesnode
        d_triesnode* node = (d_triesnode*)T;
        if (node->child[(int)c] == NULL) {
            node->child[(int)c] = (d_triesnode*)init_func();
        }
    } else if (init_func == CREATE_FILE) {
        // Handle as f_triesnode
        f_triesnode* node = (f_triesnode*)T;
        if (node->child[(int)c] == NULL) {
        fflush(stdout);
            node->child[(int)c] = (f_triesnode*)init_func();
        }
    }
}


d_triesnode* insert_directory(d_triesnode* T, char* str,int ss_id){
    d_triesnode* temp = T;
    for(int i = 0;i < strlen(str);i++){
        insert_char(temp,str[i], CREATE_DIRECTORY);
        temp = temp->child[(int)str[i]];

    }
    if(temp->directory){
        printf("Error same directory \"%s\" already exists\n", str);
        return NULL;
    }
    temp->files = init_f_trie();
    temp->directory = (file_info*)malloc(sizeof(file_info));
    // temp->directory->name = (char*)malloc(sizeof(char)*strlen(str));
    strcpy(temp->directory->name, str);

    return temp;

}

void insert_file(d_triesnode* T, char* str,int ss_id){
    if(T==NULL) return;
    f_triesnode* temp = T->files;

    for(int i = 0;i < strlen(str);i++){
        insert_char(temp,str[i], CREATE_FILE);

        temp=temp->child[(int)str[i]];
        // temp->word = insert_target(temp->word,target_word);
    }
    if(temp->file){
        printf("Error same file \"%s\" already exists\n", str);
        return;
    }

    temp->file = (file_info*)malloc(sizeof(file_info));
    // temp->file->name = (char*)malloc(sizeof(char)*strlen(str));
    strcpy(temp->file->name, str);

}


void find_prefix(d_triesnode* T, char *str){
    d_triesnode* temp = T;
    for(int i = 0;i < strlen(str);i++){
        if(temp->child[(int)str[i]])
        temp=temp->child[(int)str[i]];
        else{ 
            return;
        }
    }
    // print_files(temp);
}

// Helper function to check if a node has any children
bool has_children(void* node, bool is_directory) {
    if (node == NULL) return false;
    
    if (is_directory) {
        d_triesnode* d_node = (d_triesnode*)node;
        for (int i = 0; i < CHAR_SET; i++) {
            if (d_node->child[i] != NULL) return true;
        }
    } else {
        f_triesnode* f_node = (f_triesnode*)node;
        for (int i = 0; i < CHAR_SET; i++) {
            if (f_node->child[i] != NULL) return true;
        }
    }
    return false;
}

// Helper function to free a directory node
void trie_free_directory_node(d_triesnode* node) {
    if (node->directory) {
        free(node->directory);
        node->directory = NULL;
    }
    free(node);
}

// Helper function to free a file node
void trie_free_file_node(f_triesnode* node) {
    if (node->file) {
        free(node->file);
        node->file = NULL;
    }
    free(node);
}

// Recursively delete a file from the trie
f_triesnode* delete_file_recursive(f_triesnode* node, const char* filename, int depth, bool* deleted) {
    if (node == NULL) return NULL;

    // If we've reached the end of the filename
    if (depth == strlen(filename)) {
        // If this node contains the file we want to delete
        if (node->file) {
            *deleted = true;
            // If the node has no children, we can delete it entirely
            if (!has_children(node, false)) {
                trie_free_file_node(node);
                return NULL;
            }
            // Otherwise, just remove the file information
            free(node->file);
            node->file = NULL;
        }
        return node;
    }

    // Recursively delete from the appropriate child
    int index = (unsigned char)filename[depth];
    node->child[index] = delete_file_recursive(node->child[index], filename, depth + 1, deleted);

    // If this node has no file info and no children, delete it
    if (node->file == NULL && !has_children(node, false)) {
        trie_free_file_node(node);
        return NULL;
    }

    return node;
}

// Recursively delete a directory from the trie
d_triesnode* delete_directory_recursive(d_triesnode* node, const char* dirname, int depth, bool* deleted) {
    if (node == NULL) return NULL;

    // If we've reached the end of the dirname
    if (depth == strlen(dirname)) {
        // If this node contains the directory we want to delete
        if (node->directory) {
            *deleted = true;
            // Delete all files in this directory first
            if (node->files) {
                // Recursively delete all files
                f_triesnode* current = node->files;
                for (int i = 0; i < CHAR_SET; i++) {
                    if (current->child[i]) {
                        bool file_deleted = false;
                        current->child[i] = delete_file_recursive(current->child[i], "", 0, &file_deleted);
                    }
                }
                free(node->files);
                node->files = NULL;
            }

            // If the node has no children, we can delete it entirely
            if (!has_children(node, true)) {
                trie_free_directory_node(node);
                return NULL;
            }
            // Otherwise, just remove the directory information
            free(node->directory);
            node->directory = NULL;
        }
        return node;
    }

    // Recursively delete from the appropriate child
    int index = (unsigned char)dirname[depth];
    node->child[index] = delete_directory_recursive(node->child[index], dirname, depth + 1, deleted);

    // If this node has no directory info and no children, delete it
    if (node->directory == NULL && !has_children(node, true)) {
        trie_free_directory_node(node);
        return NULL;
    }

    return node;
}

// Public interface for deleting a file
bool delete_file(d_triesnode* T, const char* filename) {
    if (T == NULL || T->files == NULL) return false;
    
    bool deleted = false;
    T->files = delete_file_recursive(T->files, filename, 0, &deleted);
    return deleted;
}

// Public interface for deleting a directory
bool delete_directory(d_triesnode* T, const char* dirname) {
    if (T == NULL) return false;
    
    bool deleted = false;
    T = delete_directory_recursive(T, dirname, 0, &deleted);
    return deleted;
}


// // // 
// // int main() {
//     d_triesnode* root = init_d_trie();

//     int dummy_ip = 1; // Placeholder IP address
//     insert_directory(root,"/", dummy_ip);

//     // Test inserting directories
//     insert_directory(root->child['/'], "docs", dummy_ip);
//     insert_directory(root->child['/'], "images", dummy_ip);

//     // Test inserting files
//     insert_file(root->child['/'], "readme.txt", dummy_ip);

//     // Print directories and files
//     printf("Directory structure:\n");
//     print_directory(root);

//     return 0;
// }