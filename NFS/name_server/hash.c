#include"hash.h"

long long int powerofk(int n){
    long long int sum=1;
    while(n--)
        sum*=3;
    return sum;
}
int sumsqASCII(char *string){
    long long int i = 0, sum = 0;
    while (string[i] != '\0')
    {
        long long int k = (int)string[i++];
        sum += ((k) * (k) * powerofk(k));
    }
    return sum%MAX_FILES;
}

void put_directory(d_node** d, char* str, d_triesnode* path, int ss_id){
    int k = sumsqASCII(str);
    if(k < 0)
    { 
        k += MAX_FILES;
    }
    if(!d)
    {
        return;
    }
    printf("%d\n", k);
    if(d[k] == NULL)
    {
        d[k] = (d_node*)malloc(sizeof(d_node));
        d[k]->next = NULL;
        d[k]->path = path;
        d[k]->ss_id = ss_id;
        d[k]->ss_bk1 = ss_id;
        d[k]->ss_bk2 = ss_id;
        d[k]->name = malloc(sizeof(directory_info));
        strcpy(d[k]->name->name, str);
        printf("z\n");
        return;
    }
    d_node* temp = d[k];
    while(temp->next!=NULL){
        temp = temp->next;
    }
    temp->next = (d_node*)malloc(sizeof(d_node));
    temp->next->next = NULL;
    temp->next->path = path;
    temp->next->ss_id = ss_id;
    temp->next->ss_bk1 = ss_id;
    temp->next->ss_bk2 = ss_id;
    temp->next->name = malloc(sizeof(directory_info));
    strcpy(temp->next->name->name, str);
    return;
}

void put_file(d_node* d, char* str, int ss_id){
    printf("%s\n",str);
    int k = sumsqASCII(str);
    if(d->files[k] == NULL){
        d->files[k] = (f_node*)malloc(sizeof(f_node));
        d->files[k]->next = NULL;
        d->files[k]->ss_id = ss_id;
        d->files[k]->name = (file_info*)malloc(sizeof(file_info));
        strcpy(d->files[k]->name->name, str);
        return;
    }
    f_node* temp = d->files[k];
    while(temp->next!=NULL){
        temp = temp->next;
    }
    temp->next = (f_node*)malloc(sizeof(f_node));
    temp->next->next = NULL;
    temp->next->name = (file_info*)malloc(sizeof(file_info));
    temp->next->ss_id = ss_id;
    strcpy(temp->next->name->name, str);
    return;
}

void list_files(d_node* root){
    hash_print_files(root->files, "/");
    hash_print_directory(root->directories, "/");
}

void hash_print_directory(d_node** root, char* path){
    char buffer[MAX_LENGTH] = {0};
    int length = 0;
    if(path){
        strcpy(buffer, path);
        length = strlen(path);
    }
    for (int i = 0; i < MAX_FILES; i++)
    {
        d_node* temp = root[i];
        while(temp){
            for(int i = 0; i < strlen(temp->name->name); i++)
            buffer[i+length] = temp->name->name[i];
            buffer[strlen(temp->name->name)+length] = '/';

            printf("%s%s%s\n",BLUE_BOLD,buffer, RESET);
            hash_print_files(root[i]->files, buffer);
            hash_print_directory(root[i]->directories, buffer);
            for(int i = 0; i < strlen(temp->name->name); i++)
            buffer[i+length] = 0;
            buffer[strlen(temp->name->name)+length] = '\0';
            temp = temp->next;
        }
    }
}

void hash_print_files(f_node**files,char* path){
    for (int i = 0; i < MAX_FILES; i++)
    {
        f_node* temp = files[i];
        while(temp){
            printf("%s%s%s%s\n",GREEN_BOLD,path, temp->name->name, RESET);
            temp = temp->next;
        }
    }
}


void send_list_files(request_t* req, d_node* root) {
    message_t msg = req->msg;
    int sock = req->sock;
    msg.type = LIST + 1;
    // Send the "Home" directory as the starting point
    snprintf(msg.data, BUFSIZE, "%sDirectory: /Home%s", BLUE_BOLD, RESET);
    send_txn(sock, &msg, sizeof(msg), 0, req,1);
    send_hash_print_files(req, root->files, "home/");
    send_hash_print_directory(req, root->directories, "home/");
}

void send_hash_print_directory(request_t* req, d_node** root, char* path) {
    message_t msg = req->msg;
    int sock = req->sock;
    req->msg.type = LIST+1;

    char buffer[MAX_LENGTH] = {0};
    int length = 0;
    if (path) {
        strcpy(buffer, path);
        length = strlen(path);
    }

    for (int i = 0; i < MAX_FILES; i++) {
        d_node* temp = root[i];
        while (temp) {
            // Append current directory name to path
            for (int j = 0; j < strlen(temp->name->name); j++) {
                buffer[j + length] = temp->name->name[j];
            }
            buffer[strlen(temp->name->name) + length] = '/';

            // Send the directory path
            snprintf(msg.data, BUFSIZE, "%sDirectory: %s%s", BLUE_BOLD, buffer, RESET);
            send_txn(sock, &msg, sizeof(msg), 0, req, 1);

            // Recurse into files and directories
            send_hash_print_files(req, root[i]->files, buffer);
            send_hash_print_directory(req, root[i]->directories, buffer);

            // Reset buffer for the next iteration
            memset(buffer + length, 0, MAX_LENGTH - length);
            temp = temp->next;
        }
    }
    perror("send - dire");
}

void send_hash_print_files(request_t* req, f_node** files, char* path) {
    req->msg.type = LIST+1;
    message_t msg = req->msg;
    int sock = req->sock;

    for (int i = 0; i < MAX_FILES; i++) {
        f_node* temp = files[i];
        while (temp) {
    perror("send - fire");
            // Send the file path
            printf("%s\n", temp->name->name);
            snprintf(msg.data, SIZE, "%sFile: %s%s%s%s", GREEN_BOLD, path, temp->name->name, RESET);
            send_txn(sock, &msg, sizeof(msg), 0, req, 1);

            temp = temp->next;
        }
    }
}

// Helper function to free a file node
void hash_free_file_node(f_node* node) {
    if (node->name) {
        free(node->name);
    }
    free(node);
}

// Helper function to free a directory node
void hash_free_directory_node(d_node* node) {
    if (node->name) {
        free(node->name);
    }
    
    // Free all files in this directory
    for (int i = 0; i < MAX_FILES; i++) {
        f_node* curr_file = node->files[i];
        while (curr_file != NULL) {
            f_node* next = curr_file->next;
            hash_free_file_node(curr_file);
            curr_file = next;
        }
    }
    
    // Free all subdirectories
    for (int i = 0; i < MAX_FILES; i++) {
        d_node* curr_dir = node->directories[i];
        while (curr_dir != NULL) {
            d_node* next = curr_dir->next;
            hash_free_directory_node(curr_dir);
            curr_dir = next;
        }
    }
    
    free(node);
}

// Remove a file from a directory
bool remove_file(d_node* directory, const char* filename) {
    if (!directory || !filename) return false;
    
    int hash = sumsqASCII((char*)filename);
    f_node* current = directory->files[hash];
    f_node* prev = NULL;
    
    // Search for the file in the linked list at the hash bucket
    while (current != NULL) {
        if (strcmp(current->name->name, filename) == 0) {
            // Found the file to remove
            if (prev == NULL) {
                // File is at the head of the list
                directory->files[hash] = current->next;
            } else {
                // File is in the middle or end of the list
                prev->next = current->next;
            }
            
            // Free the memory
            hash_free_file_node(current);
            return true;
        }
        prev = current;
        current = current->next;
    }
    
    return false; // File not found
}

// Helper function to check if directory is empty
bool is_directory_empty(d_node* dir) {
    // Check for files
    for (int i = 0; i < MAX_FILES; i++) {
        if (dir->files[i] != NULL) return false;
    }
    
    // Check for subdirectories
    for (int i = 0; i < MAX_FILES; i++) {
        if (dir->directories[i] != NULL) return false;
    }
    
    return true;
}

// Remove a directory from the directory structure
bool remove_directory(d_node** parent_directories, const char* dirname) {
    if (!parent_directories || !dirname) return false;
    
    int hash = sumsqASCII((char*)dirname);
    d_node* current = parent_directories[hash];
    d_node* prev = NULL;
    
    // Search for the directory in the linked list at the hash bucket
    while (current != NULL) {
        if (strcmp(current->name->name, dirname) == 0) {
            // Check if directory is empty
            if (!is_directory_empty(current)) {
                printf("Error: Directory '%s' is not empty\n", dirname);
                return false;
            }
            
            // Found the directory to remove
            if (prev == NULL) {
                // Directory is at the head of the list
                parent_directories[hash] = current->next;
            } else {
                // Directory is in the middle or end of the list
                prev->next = current->next;
            }
            
            // Free the memory
            hash_free_directory_node(current);
            return true;
        }
        prev = current;
        current = current->next;
    }
    
    return false; // Directory not found
}

// Helper function to recursively remove a directory and all its contents
bool remove_directory_recursive(d_node** parent_directories, const char* dirname) {
    if (!parent_directories || !dirname) return false;
    
    int hash = sumsqASCII((char*)dirname);
    d_node* current = parent_directories[hash];
    d_node* prev = NULL;
    
    while (current != NULL) {
        if (strcmp(current->name->name, dirname) == 0) {
            // Found the directory to remove
            
            // First remove all files
            for (int i = 0; i < MAX_FILES; i++) {
                while (current->files[i] != NULL) {
                    f_node* next = current->files[i]->next;
                    hash_free_file_node(current->files[i]);
                    current->files[i] = next;
                }
            }
            
            // Then recursively remove all subdirectories
            for (int i = 0; i < MAX_FILES; i++) 
            {
                d_node* subdir = current->directories[i];
                while (subdir != NULL) 
                {
                    d_node* next = subdir->next;
                    remove_directory_recursive(&current->directories[i], subdir->name->name);
                    subdir = next;
                }
            }
            
            // Finally remove the directory itself
            if (prev == NULL) {
                parent_directories[hash] = current->next;
            } else {
                prev->next = current->next;
            }
            
            hash_free_directory_node(current);
            return true;
        }
        prev = current;
        current = current->next;
    }
    
    return false;
}
