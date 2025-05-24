#include"zip.h"


void create_directory(const char *path) {
    char temp[1024];
    snprintf(temp, sizeof(temp), "%s", path);
    size_t len = strlen(temp);

    if (temp[len - 1] == '/')
        temp[len - 1] = '\0';

    char *p = NULL;
    for (p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(temp, 0755); // Create intermediate directories
            *p = '/';
        }
    }
    mkdir(temp, 0755); // Create the final directory
}

// Function to extract a file from the ZIP archive
void extract_file(zip_t *zip, zip_uint64_t index, const char *destination) {
    struct zip_stat stat;
    zip_stat_index(zip, index, 0, &stat);

    // Create the full path for the file or directory
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", destination, stat.name);

    if (stat.name[strlen(stat.name) - 1] == '/') {
        // It's a directory, create it
        create_directory(full_path);
    } else {
        // It's a file, extract it
        zip_file_t *file = zip_fopen_index(zip, index, 0);
        if (!file) {
            fprintf(stderr, "Error opening file in zip: %s\n", zip_strerror(zip));
            return;
        }

        // Create parent directories if needed
        char *last_slash = strrchr(full_path, '/');
        if (last_slash) {
            *last_slash = '\0';
            create_directory(full_path);
            *last_slash = '/';
        }

        FILE *output = fopen(full_path, "wb");
        if (!output) {
            perror("fopen");
            zip_fclose(file);
            return;
        }

        char buffer[4096];
        zip_int64_t bytes_read;
        while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
            fwrite(buffer, 1, bytes_read, output);
        }

        fclose(output);
        zip_fclose(file);
    }
}

// Function to unzip the archive
void unzip(const char *zip_path, const char *destination) {
    int error;
    zip_t *zip = zip_open(zip_path, 0, &error);
    if (!zip) {
        fprintf(stderr, "Error opening zip file: %s\n", zip_strerror(zip));
        return;
    }

    // Create the destination directory if it doesn't exist
    create_directory(destination);

    zip_uint64_t num_entries = zip_get_num_entries(zip, 0);
    for (zip_uint64_t i = 0; i < num_entries; i++) {
        extract_file(zip, i, destination);
    }

    zip_close(zip);
    printf("Successfully extracted %s to %s\n", zip_path, destination);
}
