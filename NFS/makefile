# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Directories
CLIENT_DIR = ./client
UTILITIES_DIR = ./utilities
NAME_SERVER_DIR = ./name_server
CLIENT_SERVER_DIR = ./client_server
STORAGE_SERVER_DIR = ./storage_server

# Source files
CLIENT_SRC = $(wildcard $(CLIENT_DIR)/*.c)
UTILITIES_SRC = $(wildcard $(UTILITIES_DIR)/*.c)
NAME_SERVER_SRC = $(wildcard $(NAME_SERVER_DIR)/*.c)
CLIENT_SERVER_SRC = $(wildcard $(CLIENT_SERVER_DIR)/*.c)
STORAGE_SERVER_SRC = $(wildcard $(STORAGE_SERVER_DIR)/*.c)

# Object files
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
UTILITIES_OBJ = $(UTILITIES_SRC:.c=.o)
NAME_SERVER_OBJ = $(NAME_SERVER_SRC:.c=.o)
CLIENT_SERVER_OBJ = $(CLIENT_SERVER_SRC:.c=.o)
STORAGE_SERVER_OBJ = $(STORAGE_SERVER_SRC:.c=.o)

# Executables
SERVER_EXEC = server.out
CLIENT_EXEC = client.out
STORAGE_EXEC = storage.out

# Default target
all: $(SERVER_EXEC) $(CLIENT_EXEC) $(STORAGE_EXEC)

# Rule to build the server executable
$(SERVER_EXEC): $(NAME_SERVER_OBJ) $(UTILITIES_OBJ) $(CLIENT_SERVER_OBJ)
	$(CC) $(CFLAGS) -o $(SERVER_EXEC) $(NAME_SERVER_OBJ) $(UTILITIES_OBJ) $(CLIENT_SERVER_OBJ) -lm -lvlc -lzip

# Rule to build the client executable
$(CLIENT_EXEC): $(CLIENT_OBJ) $(UTILITIES_OBJ)
	$(CC) $(CFLAGS) -o $(CLIENT_EXEC) $(CLIENT_OBJ) $(UTILITIES_OBJ) -lm -lvlc -lzip

# Rule to build the storage executable
$(STORAGE_EXEC): $(STORAGE_SERVER_OBJ) $(UTILITIES_OBJ)
	$(CC) $(CFLAGS) -o $(STORAGE_EXEC) $(STORAGE_SERVER_OBJ) $(UTILITIES_OBJ) -lm -lvlc -lzip

# Clean rule
clean:
	rm -f $(CLIENT_OBJ) $(UTILITIES_OBJ) $(NAME_SERVER_OBJ) $(CLIENT_SERVER_OBJ) $(STORAGE_SERVER_OBJ) $(SERVER_EXEC) $(CLIENT_EXEC) $(STORAGE_EXEC)

# Run the server
run_server: $(SERVER_EXEC)
	./$(SERVER_EXEC)

# Run the storage
run_storage: $(STORAGE_EXEC)
	./$(STORAGE_EXEC)

.PHONY: all clean run_server run_storage