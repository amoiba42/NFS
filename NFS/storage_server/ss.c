#include "ss.h"

logfile_t* logfile;
char PATH[SIZE];
int main(int argc, char* argv[])
{
    // Takes accessible path as command line argument

    #ifdef LOG
    if (argc < 5) 
    {
        fprintf(stderr, "usage: %s <path> <logfile>\n", argv[0]);
        exit(1);
    }
    logfile = (logfile_t*) calloc(1, sizeof(logfile_t));
    strcpy(logfile->path, argv[4]);
    pthread_mutex_init(&(logfile->lock), NULL);

    #else
    if (argc < 2) 
    {
        fprintf(stderr, "usage: %s <path>\n", argv[0]);
        exit(1);
    }
    logfile = (logfile_t*) calloc(1, sizeof(logfile_t));
    logfile->path[0] = 0;
    pthread_mutex_init(&(logfile->lock), NULL);
    #endif

    // signal(SIGPIPE, SIG_IGN);
    char NS_ip[16];
    strcpy(NS_ip, argv[3]);
    char* local_ip = get_local_ip();
    int nsport = atoi(argv[2]);
    int clport = nsport - 1;
    int stport = nsport - 2;

    // char path[SIZE];
    strcpy(PATH, argv[1]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NAME_PORT);
    addr.sin_addr.s_addr = inet_addr(NS_ip);

    connect(sock, (struct sockaddr*) &addr, sizeof(addr));
    logst(STATUS, "Connected to the naming server");

    message_t msg = 
    {
        .type = JOIN,
    };
    strncpy(msg.recv_ip.ip, local_ip, sizeof(msg.recv_ip.ip) - 1);
    msg.recv_ip.port = nsport;
    strcpy(msg.path, PATH);

    send(sock, &msg, sizeof(msg), 0);
    logst(STATUS, "Sent registration request to the naming server");

    message_t response;
    recv(sock, &response, sizeof(response), 0);
    if (response.type == JOIN + 1) 
    {
        logst(COMPLETION, "Registered with the naming server");
    } 
    else 
    {
        // logst(ERROR, "Registration failed: %s", response.data);
        close(sock);
        exit(EXIT_FAILURE);
    }

    close(sock);

    pthread_t server, client, storage;

    listener_args_t ns_args;
    ns_args.ip = local_ip; 
    ns_args.port = nsport;         

    listener_args_t cl_args;
    cl_args.ip = local_ip;    
    cl_args.port = clport;       

    listener_args_t st_args;
    st_args.ip = local_ip;      
    st_args.port = stport;

    pthread_create(&server, NULL, ns_listener, &ns_args);
    pthread_create(&client, NULL, cl_listener, &cl_args);
    pthread_create(&storage, NULL, st_listener, &st_args);


    logst(STATUS, "Started listener threads");

    pthread_join(server, NULL);
    pthread_join(client, NULL);
    pthread_join(storage, NULL);

    return 0;
}   