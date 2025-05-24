#include"zip.h"
#include"ss.h"

void add_file_to_zip(zip_t *zip, const char *file_path, const char *zip_path) {
    // Open the file to be added
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("Unable to open file");
        return;
    }

    // Read the file contents
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_data = malloc(file_size);
    fread(file_data, 1, file_size, file);
    fclose(file);

    // Add the file to the zip archive
    zip_source_t *source = zip_source_buffer(zip, file_data, file_size, 1);
    if (source == NULL) {
        fprintf(stderr, "Error creating zip source for file %s: %s\n", file_path, zip_strerror(zip));
        free(file_data);
        return;
    }

    if (zip_file_add(zip, zip_path, source, ZIP_FL_OVERWRITE) < 0) {
        fprintf(stderr, "Error adding file %s to zip: %s\n", zip_path, zip_strerror(zip));
        zip_source_free(source);
        return;
    }
}

void zip_directory(zip_t *zip, const char *dir_path, const char *base_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Unable to open directory");
        return;
    }

    struct dirent *entry;
    char file_path[1024];
    char zip_path[1024];

    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);
        snprintf(zip_path, sizeof(zip_path), "%s/%s", base_path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            // Recursively add directories
            zip_directory(zip, file_path, zip_path);
        } else {
            // Add file to zip
            add_file_to_zip(zip, file_path, zip_path);
        }
    }

    closedir(dir);
}

void send_file(int socket, const char *file_path, message_t *msg, request_t *req) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror_tpx(req, "File open");
        return;
    }

    // Initialize message type for sending file data
    msg->type = COPY_ACROSS;

    char buffer[SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, SIZE, file)) > 0) {
        // Copy the read data into the message's data field
        bzero(msg->data, SIZE);
        memcpy(msg->data, buffer, bytes_read);

        booking(&msg);// Send the message
        if (send_txn(socket, msg, sizeof(*msg), 0, req, 1) < 0) {
            perror_tpx(req, "send_txn");
            fclose(file);
            return;
        }
    }
    fclose(file);

    // Send a STOP message to indicate the end of the file transfer
    msg->type = STOP;
    bzero(msg->data, SIZE);
    booking(&msg);
    if (send_txn(socket, msg, sizeof(message_t), 0, req, 1) < 0) {
        perror_tpx(req, "send_txn");
    }

    // Log completion
    logst(COMPLETION, "Finished sending file %s", file_path);
}

