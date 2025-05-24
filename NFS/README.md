# NFS Client Side Implementation

## Overview

This project implements the client-side of a Network File System (NFS). The client communicates with a naming server to perform various file operations such as reading, writing, creating, copying, deleting, streaming, listing, and retrieving file information. The implementation is designed to interact with a server to manage files remotely while maintaining simplicity and efficiency.

---

## Features

The client supports the following operations:
- **Read (`R`)**: Retrieve the contents of a file from the server.
- **Write (`W`)**: Send data to be written to a file on the server.
- **Create (`C`)**: Create a new file or directory on the server.
- **Delete (`D`)**: Remove a file or directory from the server.
- **Copy (`P`)**: Duplicate a file on the server from one path to another.
- **Stream (`S`)**: Stream data from the server.
- **List (`L`)**: List all files available on the server.
- **Info (`I`)**: Get metadata or information about a specific file or directory.
- **Exit (`Q`)**: Quit the client.

---

## File Structure

### 1. **`client.h`**
   - Defines the necessary headers, macros, and function prototypes for the client.
   - Provides external linkage for the global `name_socket`.

### 2. **Main Client Program**
   - **Initialization**:
     - Handles optional logging setup.
     - Initializes mutexes for thread-safe logging operations.
   - **Connection**:
     - Establishes a socket connection to the naming server using `AF_INET` protocol.
     - Configures the socket with the naming server's IP and port.
   - **User Interaction**:
     - Displays a menu to the user for selecting operations.
     - Processes user input and calls appropriate request functions.
   - **Termination**:
     - Safely closes the socket, destroys mutexes, and frees allocated resources.

### 3. **Request Functions**
   - Functions like `request_create`, `request_delete`, and `request_info` encapsulate the logic for each operation.
   - These functions:
     - Create and populate a `message_t` structure to send requests to the server.
     - Use `send` and `recv` to communicate with the server.
     - Handle responses from the server and log the results.

#### Example: **`request_create`**
   - **Purpose**: Create a new file or directory on the server.
   - The user is prompted to specify whether the request is for a file or a directory.
   - Sends the appropriate request and logs success or failure based on the server's response.

#### Example: **`request_delete`**
   - **Purpose**: Delete a file or directory from the server.
   - The user specifies the file or directory to delete.
   - Handles error cases such as file not found, file being read or written, or permission issues.

#### Example: **`request_info`**
   - **Purpose**: Retrieve metadata (such as server details) about a file or directory.
   - The user provides a path and file name to request information.
   - The client connects to a storage server to get the requested information and displays it.

---

## Key Concepts

1. **Communication Protocol**:
   - Messages are exchanged between the client and server using a `message_t` structure.
   - Each operation corresponds to a specific message type (`CREATE`, `DELETE`, `INFO`, etc.), ensuring clarity and separation of concerns.

2. **Thread-Safe Logging**:
   - Logging is synchronized using `pthread_mutex` to avoid race conditions when multiple threads attempt to write to the log file simultaneously.

3. **Error Handling**:
   - Comprehensive error messages for socket creation, connection, and communication failures.
   - Response types such as `NOTFOUND`, `EXISTS`, `PERM`, and `UNAVAILABLE` are handled gracefully.

---


