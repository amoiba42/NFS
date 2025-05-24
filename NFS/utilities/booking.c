#include "utilities.h"

void booking(message_t* msg) {
    const char *file_path = "booking.txt"; // Hardcoded file path
    FILE *file = fopen(file_path, "a"); // Open the file in append mode
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    char* type = (char*)malloc(20*sizeof(char));
    if(msg->type == 0 || msg->type == 1)
        strcpy(type, "READ");
    if(msg->type == 2 || msg->type == 3)
        strcpy(type, "WRITE");
    if(msg->type == 4 || msg->type == 5)
        strcpy(type, "CREATE_F");
    if(msg->type == 6 || msg->type == 7)
        strcpy(type, "CREATE_D");
    if(msg->type == 10 || msg->type == 11)
        strcpy(type, "LIST");
    if(msg->type == 12|| msg->type == 13)
        strcpy(type, "INFO");
    if(msg->type == 14 || msg->type == 15)
        strcpy(type, "COPY");
    if(msg->type == 16 || msg->type == 17)
        strcpy(type, "COPY_INTERNAL");
    if(msg->type == 18 || msg->type == 19)
        strcpy(type, "ACROSS");
    if(msg->type == 20 || msg->type == 21)
        strcpy(type, "BACKUP");
    if(msg->type == 22 || msg->type == 23)
        strcpy(type, "UPDATE");
    if(msg->type == 24 || msg->type == 25)
        strcpy(type, "JOIN");
    if(msg->type == 26 || msg->type == 27)
        strcpy(type, "PING");
    if(msg->type == 28 || msg->type == 29)
        strcpy(type, "STOP");
    if(msg->type == 30 || msg->type == 31)
        strcpy(type, "STREAM");
    if(msg->type == 40 || msg->type == 41)
        strcpy(type, "DELETE_F");
    if(msg->type == 42 || msg->type == 43)
        strcpy(type, "DELETE_D");
    if(msg->type == -1)
        strcpy(type, "INVALID");
    if(msg->type == -2)
        strcpy(type, "NOTFOUND");
    if(msg->type == -3)
        strcpy(type, "EXISTS");
    if(msg->type == -4)
        strcpy(type, "BEING READ");
    if(msg->type == -5)
        strcpy(type, "RDONLY");
    if(msg->type == -6)
        strcpy(type, "XLOCK");
    if(msg->type == -7)
        strcpy(type, "PERM");
    if(msg->type == -8)
        strcpy(type, "UNAVAILABLE");
    // Append the message content to the file
    fprintf(file, "Message Type: %s\n", type);
    fprintf(file, "Data: %s\n", msg->data);
    fprintf(file, "Receiver IP: %s:%u\n", msg->recv_ip.ip, msg->recv_ip.port);
    fprintf(file, "Sender IP: %s:%u\n", msg->send_ip.ip, msg->send_ip.port);
    fprintf(file, "Path: %s\n", msg->path);
    fprintf(file, "File Name: %s\n", msg->file_name);
    fprintf(file, "---------------------------------------\n");

    fclose(file); // Close the file
}
