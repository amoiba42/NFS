#ifndef ZIP_UTILS_H
#define ZIP_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zip.h>
#include "ss.h"


/**
 * @brief Adds a file to the zip archive.
 *
 * @param zip Pointer to the zip archive object.
 * @param file_path Path to the file to be added to the zip.
 * @param zip_path Path inside the zip archive for the file.
 */
void add_file_to_zip(zip_t *zip, const char *file_path, const char *zip_path);

/**
 * @brief Recursively adds a directory and its contents to the zip archive.
 *
 * @param zip Pointer to the zip archive object.
 * @param dir_path Path to the directory to be added to the zip.
 * @param base_path Base path inside the zip archive for the directory contents.
 */
void zip_directory(zip_t *zip, const char *dir_path, const char *base_path);


// #define BUFFER_SIZE 4096

/**
 * @brief Creates a directory along with any necessary parent directories.
 *
 * @param path The directory path to be created.
 */
void create_directory(const char *path);

/**
 * @brief Extracts a specific file from a ZIP archive.
 *
 * @param zip Pointer to the opened zip archive.
 * @param index Index of the file in the zip archive.
 * @param destination The destination directory where the file will be extracted.
 */
void extract_file(zip_t *zip, zip_uint64_t index, const char *destination);

/**
 * @brief Extracts all files from a ZIP archive into a specified destination directory.
 *
 * @param zip_path Path to the zip archive to be extracted.
 * @param destination Directory where the files will be extracted.
 */
void unzip(const char *zip_path, const char *destination);
void send_file(int socket, const char *file_path, message_t *msg, request_t *req);

#endif // ZIP_UTILS_H
