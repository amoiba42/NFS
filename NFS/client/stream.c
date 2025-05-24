#include "client.h"
#include "defs.h"
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
// #define SIZE 4096

void play_audio_from_stream(int socket_fd) {
    int pipe_fd[2];

    // Create a pipe
    if (pipe(pipe_fd) == -1) {
        perror("Pipe failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process: Redirect stdin to the pipe and play with MPV
        close(pipe_fd[1]); // Close the write end of the pipe
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]); // Close the read end after duplicating

        // Execute MPV
        execlp("mpv", "mpv", "--no-video", "-", NULL);
        perror("Failed to execute mpv");
        exit(EXIT_FAILURE);
    } else {
        // Parent process: Receive data and write to the pipe
        close(pipe_fd[0]); // Close the read end of the pipe

        char buffer[SIZE];
        ssize_t bytes_received;

        while ((bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0)) > 0) {
            if (write(pipe_fd[1], buffer, bytes_received) < 0) {
                perror("Failed to write to pipe");
                break;
            }
        }

        close(pipe_fd[1]); // Close the write end of the pipe
        wait(NULL);        // Wait for the child process to finish
    }
}


void request_stream(int name_socket)
{
    message_t msg;
    msg.type = STREAM;
    memset(msg.data, 0, SIZE);
    memset(msg.path, 0, PATH_MAX);

    // char *path = "/brooklyn_nine_nine.mp3"; // Hardcoded path to the audio file
    // strcpy(msg.path, path);

    char *path = (char *)malloc(SIZE * sizeof(char));
    perror("Path: ");
    scanf(" %[^\n]s", path);
    strcpy(msg.path, path);

    char *file_name = (char *)malloc(SIZE * sizeof(char));
    perror("File: ");
    scanf(" %[^\n]s", file_name);
    strcpy(msg.file_name, file_name);

    send_txn(name_socket, &msg, sizeof(message_t), 0, NULL, 0);
    recv_txn(name_socket, &msg, sizeof(message_t), 0, NULL, 0);
    perror("sent request\n");

    int port;
    char ip[INET_ADDRSTRLEN];
    printf("msg.type is %d\n", msg.type);
    if (msg.type == STREAM + 1)
    {
        strncpy(ip, msg.send_ip.ip, INET_ADDRSTRLEN);
        port = msg.send_ip.port;
        logc(PROGRESS, "Sending request to storage server %s:%d", ip, port);
    }
    else if (msg.type == NOTFOUND)
    {
        logc(FAILURE, "%s was not found", path);
        return;
    }
    else if (msg.type == UNAVAILABLE)
    {
        logc(FAILURE, "%s is unavailable currently", path);
        return;
    }
    else if (msg.type == XLOCK)
    {
        logc(FAILURE, "%s is being written to by a client", path);
        return;
    }
    else if (msg.type == PERM)
    {
        logc(FAILURE, "Missing permissions for read");
        return;
    }
    else
    {
        logc(FAILURE, "Unexpected response from server");
        return;
    }
    perror("done naming now connecting to storage\n");
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    int ss_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (ss_sock < 0)
    {
        perror_tx("socket");
        return;
    }

    if (connect(ss_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror_tx("Unable to connect to Storage Server");
        close(ss_sock);
        return;
    }

    // Send stream request to storage server
    msg.type = STREAM;
    strcpy(msg.path, path);
    if (send(ss_sock, &msg, sizeof(message_t), 0) < 0)
    {
        perror_tx("send");
        close(ss_sock);
        return;
    }

    if (recv(ss_sock, &msg, sizeof(message_t), 0) < 0)
    {
        perror_tx("recv");
        close(ss_sock);
        return;
    }

    int bytes;
    if (msg.type == STREAM + 1)
    {
        bytes = atoi(msg.data);
        logc(PROGRESS, "Receiving %d bytes from storage server %s:%d", bytes, ip, port);
    }
    else if (msg.type == NOTFOUND)
    {
        logc(FAILURE, "%s was not found", path);
        goto ret;
    }
    else if (msg.type == PERM)
    {
        logc(FAILURE, "Missing permissions for read");
        goto ret;
    }
    else
    {
        logc(FAILURE, "Invalid server response: %d", msg.type);
        goto ret;
    }

    logc(PROGRESS, "Starting audio stream from %s:%d", ip, port);

    play_audio_from_stream(ss_sock);

    // FILE *mpv_pipe = popen("mpv --no-terminal --quiet --", "w");
    // if (!mpv_pipe)
    // {
    //     perror_tx("popen");
    //     close(ss_sock);
    //     return;
    // }
    // perror("ok6\n");

    // uint8_t buffer[4096];
    // ssize_t bytes_read;
    // while ((bytes_read = recv(ss_sock, &msg, sizeof(msg), 0)) > 0)
    // {
    //     perror("ok bharti");
    //     if(msg.type==STREAM + 1){
    //         if (fwrite(msg.data, 1, bytes_read, mpv_pipe) != bytes_read)
    //         {
    //             perror("fwrite");
    //             break;
    //         }
    //     }
    //     else if(msg.type == STOP){
    //         break;
    //     }

    // }
    // perror("ok7\n");

    // if (bytes_read < 0)
    // {
    //     perror_tx("recv");
    // }
    // pclose(mpv_pipe);
    // close(ss_sock);
    // if (bytes_read < 0)
    // {
    //     perror_tx("recv");
    // }
    // pclose(mpv_pipe);
    // close(ss_sock);

ret:
    // fclose(file);
    // free(path);
    // free(localpath);
    // free(ip);
    close(ss_sock);
}
