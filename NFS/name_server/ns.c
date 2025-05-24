#include "ns.h"

logfile_t* logfile;
d_triesnode* root_trie;
d_node* root_map;
storage_t* server_registry = NULL;
LRUCache cache;
int ss_count = 0;

pthread_mutex_t registry_lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[])
{
    #ifdef LOG
    if (argc < 2) 
    {
        fprintf(stderr, "usage: %s <logfile>\n", argv[0]);
        exit(1);
    }

    logfile = (logfile_t*) calloc(1, sizeof(logfile_t));
    strcpy(logfile->path, argv[1]);
    pthread_mutex_init(&(logfile->lock), NULL);

    #else
    logfile = (logfile_t*) calloc(1, sizeof(logfile_t));
    logfile->path[0] = 0;
    pthread_mutex_init(&(logfile->lock), NULL);

    #endif
    
    root_trie = (d_triesnode*)CREATE_DIRECTORY();
    root_trie->files = init_f_trie();
    root_trie->directory = (file_info*)malloc(sizeof(file_info));
    // root_trie->directory->name = (char*)malloc(sizeof(char)*1024);
    strcpy(root_trie->directory->name, "home");
    strcpy(root_trie->directory->path, "home");
    // root_trie->directory->path[0] = "home";
    root_map = (d_node*)malloc(sizeof(d_node));
    root_map->name = root_trie->directory;
    root_map->path = root_trie;
    root_map->next = NULL;

    // signal_tx(SIGPIPE, SIG_IGN);
  pthread_t check_backup;
  pthread_create_tx(&check_backup, NULL, handle_backup, NULL);
  pthread_t check_ss;
  pthread_create_tx(&check_ss, NULL, ns_ping, NULL);
  

    pthread_mutex_t registry_lock = PTHREAD_MUTEX_INITIALIZER;

    NAME_IP = get_local_ip();
    printf("%s\n", NAME_IP);
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NAME_PORT);
    addr.sin_addr.s_addr = inet_addr(NAME_IP);

    if(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0){
        perror("bind");
        return 0;
    }
    listen(sock, 64);

    while(1)
    {
        pthread_t worker;

        request_t* req = (request_t*) calloc(1, sizeof(request_t));
        if (req == 0)
        {
            perror_tx("calloc");
        }
        req->newsock = -1;
        req->addrlen = sizeof(req->addr);
        req->sock = accept(sock, (struct sockaddr*) &(req->addr), &(req->addrlen));

        char ip[INET_ADDRSTRLEN];
        int port = ntohs(req->addr.sin_port);
        inet_ntop(AF_INET, &(req->addr.sin_addr), ip, INET_ADDRSTRLEN);
        logns(EVENT, "Accepted connection from %s:%d", ip, port);

        pthread_create(&worker, NULL, handler_thread, req);
    }
}

void* handler_thread(void* arg)
{
    request_t* req = arg;
    recv(req->sock, &(req->msg), sizeof(req->msg), 0);

    if (req->msg.type == CREATE_D)
    {
        ns_create_d(req);
    }
    else if (req->msg.type == CREATE_F)
    {
        ns_create_f(req);
    }
    else if (req->msg.type == DELETE_D)
    {
        ns_delete_d(req);
    }
    else if (req->msg.type == DELETE_F)
    {
        ns_delete_f(req);
    }
    else if (req->msg.type == READ)
    {
        ns_read(req);
    }
    else if (req->msg.type == WRITE)
    {
        ns_write(req);
    }
    else if (req->msg.type == LIST)
    {
        ns_list(req);
    }
    else if (req->msg.type == INFO)
    {
        ns_give_info(req);
    }
    else if(req->msg.type == COPY)
    {
        ns_copy(req);
    }
    else if(req->msg.type == COPY_INTERNAL)
    {
        ns_copy_internal(req);
    }
    else if(req->msg.type == COPY_ACROSS)
    {
        ns_copy_across(req);
    }
    else if(req->msg.type == BACKUP)
    {
        ns_backup(req);
    }
    else if(req->msg.type == UPDATE)
    {
        ns_update(req);
    }
    else if(req->msg.type == JOIN)
    {
        ns_join(req);
    }
    else if(req->msg.type == STOP)
    {
        ns_stop(req);
    }
    else if(req->msg.type == STREAM)
    {
        ns_stream(req);
    }
    else
    {
        ns_handle_error(req);
    }

    return NULL;
}