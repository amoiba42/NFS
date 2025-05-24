#include "ss.h"
#include"zip.h"
void* ss_handle_copy_across(void* arg)
{
    request_t* r = arg;
    message_t msg = r->msg;
    int sock = r->sock;

    logst(EVENT, "Received copy file request from naming server, for %s", msg.data);

    char* current = (char*)malloc(PATH_MAX * sizeof(char));
    char* new = (char*)malloc(PATH_MAX * sizeof(char));
    char* ip = (char*)malloc(INET_ADDRSTRLEN * sizeof(char));
    int port;

    sprintf(current, "%s", msg.path);
    recv_txn(sock, &msg, sizeof(message_t), 0, r, 1);
    sprintf(new, "%s", msg.path);

    recv_txn(sock, &msg, sizeof(message_t), 0, r, 1);
    strcpy(ip, msg.recv_ip.ip);
    port = msg.recv_ip.port;

    struct stat state;
    if (stat(current, &state) < 0)
    {
        msg.type = UNAVAILABLE;
        booking(&msg);
        send_txn(sock, &msg, sizeof(message_t), 0, r, 1);
        reqfree(r);
        return NULL;
    }
    else
    {
        struct sockaddr_in addr;
        memset(&addr, '\0', sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip);

        logst(PROGRESS, "Connecting to %s:%d, for copy of %s", ip, port, msg.data);

        int ss_sock = socket(AF_INET, SOCK_STREAM, 0);
        r->newsock = ss_sock;
        connect(ss_sock, (struct sockaddr*)&addr, sizeof(addr));
        msg.type = COPY_ACROSS;
        strcpy(msg.path, new);
        booking(&msg);
        send_txn(ss_sock, &msg, sizeof(msg), 0, r, 0);
        // const char *current = argv[1];
        const char *zip_path = "temp.zip";
        // Open the zip file for writing
        int error;
        zip_t *zip = zip_open(zip_path, ZIP_CREATE | ZIP_TRUNCATE, &error);
        if (!zip) {
            fprintf(stderr, "Error creating zip file: %s\n", zip_strerror(zip));
            return 1;
        }

        // Check if the source is a directory or a file
        DIR *dir = opendir(current);
        if (dir) {
            // Source is a directory
            zip_directory(zip, current, "");
            closedir(dir);
        } else {
            // Source is a single file
            add_file_to_zip(zip, current, zip_path);
        }

        // Close the zip archive
        if (zip_close(zip) < 0) {
            fprintf(stderr, "Error closing zip file: %s\n", zip_strerror(zip));
            return 1;
        }
        send_file(ss_sock, "temp.zip", &msg, r);
        printf("File sent to server.\n");
    }
    msg.type = COPY_ACROSS + 1;
    booking(&msg);
    send_txn(sock, &msg, sizeof(message_t), 0, r, 1);
    reqfree(r);
    return NULL;
}

void ss_handle_copy_recv(int client_socket, request_t *req) {
    message_t msg = req->msg;
    FILE *file = NULL;
    char path[PATH_MAX];  // Temporary file path
    strcpy(path,msg.path);
    // Log the incoming request
    char client_ip[INET_ADDRSTRLEN];
    int client_port = ntohs(req->addr.sin_port);
    if (inet_ntop(AF_INET, &(req->addr.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL) {
        perror_tpx(req, "inet_ntop");
        reqfree(req);
        return;
    }

    logst(EVENT, "Received copy file request from %s:%d", client_ip, client_port);

    // Start receiving file size and content
    recv_txn(client_socket, &msg, sizeof(message_t), 0, req, 1);
    int bytes = atoi(msg.data); // Get file size from the initial message

    // File handling
    file = fopen("temp.zip", "wb");
    if (!file) {
        perror_tpx(req, "File open");
        msg.type = UNAVAILABLE;
        booking(&msg);
        send_txn(client_socket, &msg, sizeof(message_t), 0, req, 1);
        reqfree(req);
        return;
    }

    // Acknowledge readiness to receive the file
    msg.type = COPY_ACROSS + 1;
    booking(&msg);
    send_txn(client_socket, &msg, sizeof(message_t), 0, req, 1);

    int bytes_left = bytes;
    logst(PROGRESS, "Receiving %d bytes from %s:%d", bytes, client_ip, client_port);

    while (bytes_left > 0) {
        recv_txn(client_socket, &msg, sizeof(message_t), 0, req, 1);

        if (msg.type == STOP) {
            fclose(file);
            logst(COMPLETION, "Received %d bytes from %s:%d", bytes, client_ip, client_port);
            msg.type = COPY_ACROSS + 1;
            booking(&msg);
            send_txn(client_socket, &msg, sizeof(message_t), 0, req, 1);
            break;
        } else if (msg.type == COPY_ACROSS) {
            int chunk_size = (bytes_left > SIZE) ? SIZE : bytes_left;
            int written = fwrite(msg.data, sizeof(char), chunk_size, file);
            if (written != chunk_size) {
                perror_tpx(req, "fwrite");
                break;
            }
            bytes_left -= chunk_size;
        } else {
            logst(FAILURE, "Unexpected message type received from %s:%d", client_ip, client_port);
            break;
        }
    }

    fclose(file);

    if (bytes_left == 0) {
        // Post-process the file (e.g., unzip)
        unzip("temp.zip", path);
        remove(path);
    }

    reqfree(req);
    return;
}

void* ss_handle_copy_internal(void* arg){
    request_t* r = arg;
    message_t msg = r->msg;
    int sock = r->sock;
    perror("here");

    logst(EVENT, "Received copy file request from naming server, for %s", msg.data);

    char* current = (char*)malloc(PATH_MAX*sizeof(char));
    char* new = (char*)malloc(PATH_MAX*sizeof(char));
    char* ip = (char*)malloc(INET_ADDRSTRLEN*sizeof(char));
    int port;

    sprintf(current, "%s", msg.path);
    recv_txn(sock, &msg, sizeof(message_t), 0, r, 1);
    sprintf(new, "%s", msg.path);
    
    
    struct stat state;

    if (stat(current, &state) < 0)
    {
        msg.type = UNAVAILABLE;
        booking(&msg);
        send_txn(sock, &msg, sizeof(message_t), 0, r, 1);
        reqfree(r);
        return NULL;
    }
    // const char *source_path = argv[1];
    const char *zip_path = "temp.zip";

    // const char *destination = argv[2];
    // Open the zip file for writing
    int error;
    zip_t *zip = zip_open(zip_path, ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!zip) {
        fprintf(stderr, "Error creating zip file: %s\n", zip_strerror(zip));
        return 1;
    }

    // Check if the source is a directory or a file
    DIR *dir = opendir(current);
    if (dir) {
        // Source is a directory
        zip_directory(zip, current, "");
        closedir(dir);
    } else {
        // Source is a single file
        add_file_to_zip(zip, current, zip_path);
    }

    // Close the zip archive
    if (zip_close(zip) < 0) {
        fprintf(stderr, "Error closing zip file: %s\n", zip_strerror(zip));
        return 1;
    }
    unzip(zip_path, new);
    remove(zip_path);
    printf("Successfully created %s\n", zip_path);
    logst(COMPLETION, "Completed copy file request, for %s", current);
    msg.type = COPY_INTERNAL + 1;
    booking(&msg);
    send_txn(sock, &msg, sizeof(message_t), 0, r, 1);
    reqfree(r);
    return NULL;
}

