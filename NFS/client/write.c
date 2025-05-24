#include "client.h"
#include "defs.h"

void request_write(int name_socket)
{
  message_t msg;
  msg.type = WRITE;
  memset(msg.data, 0, SIZE);
  memset(msg.path, 0, PATH_MAX);

  char *path = (char *)malloc(SIZE * sizeof(char));
  printf("Path: ");
  scanf(" %[^\n]s", path);
  strcpy(msg.path, path);

  char *file_name = (char *)malloc(SIZE * sizeof(char));
  printf("FileName: ");
  scanf(" %[^\n]s", file_name);
  strcpy(msg.file_name, file_name);

  char *localpath = (char *)malloc(SIZE * sizeof(char));
  printf("Local Path: ");
  scanf(" %[^\n]s", localpath);

  booking(&msg);
  if (send(name_socket, &msg, sizeof(message_t), 0) < 0)
  {
    perror_tx("send");
    return;
  }
  if (recv(name_socket, &msg, sizeof(message_t), 0) < 0)
  {
    perror_tx("recv");
    return;
  }

  char *ip = (char *)malloc(INET_ADDRSTRLEN * sizeof(char));
  int port;

  if (msg.type == WRITE + 1)
  {
    strncpy(ip, msg.send_ip.ip, INET_ADDRSTRLEN);
    port = (msg.send_ip.port);
    logc(PROGRESS, "Sending request to storage server %s:%d", ip, port);
    goto connect;
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
  else if (msg.type == BEING_READ)
  {
    logc(FAILURE, "%s is being read currently", path);
    return;
  }
  else if (msg.type == RDONLY)
  {
    logc(FAILURE, "%s has been marked read-only", path);
    return;
  }
  else if (msg.type == XLOCK)
  {
    logc(FAILURE, "%s is being written to by a client", path);
    return;
  }
  else if (msg.type == PERM)
  {
    logc(FAILURE, "Missing permissions for write");
    return;
  }
  else
  {
    logc(FAILURE, "Received an invalid response %d from the server", msg.type);
    return;
  }

connect:
  int ss_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (ss_sock < 0)
  {
    perror_tx("sock");
    return;
  }

  struct sockaddr_in addr;
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip);
  if (connect(ss_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror_tx("Unable to connect to Storage Server");
    return;
  }

  msg.type = WRITE;
  strcpy(msg.path, path);
  strcpy(msg.file_name, file_name);
  booking(&msg);
  if (send(ss_sock, &msg, sizeof(message_t), 0) < 0)
  {
    perror_tx("send");
    return;
  }
  if (recv(ss_sock, &msg, sizeof(message_t), 0) < 0)
  {
    perror_tx("recv");
  }

  FILE *file = fopen(localpath, "r");
  // if (file == NULL)
  // {
  //   perror_tpx(req, "fopen");
  //   return;
  // }
  struct stat st;
  stat(localpath, &st);
  int bytes = st.st_size;

  if (msg.type == WRITE + 1)
  {
    logc(PROGRESS, "Sending %d bytes to storage server %s:%d", bytes, ip, port);
    goto send;
  }
  else if (msg.type == NOTFOUND)
  {
    logc(FAILURE, "%s was not found", path);
    goto ret;
  }
  else if (msg.type == RDONLY)
  {
    logc(FAILURE, "%s has been marked read-only", path);
    goto ret;
  }
  else if (msg.type == XLOCK)
  {
    logc(FAILURE, "%s is being written to by a client", path);
    goto ret;
  }
  else if (msg.type == PERM)
  {
    logc(FAILURE, "Missing permissions for write");
    goto ret;
  }
  else
  {
    logc(FAILURE, "Received an invalid response %d from the server", msg.type);
    goto ret;
  }

send:
  msg.type = WRITE;
  sprintf(msg.data, "%d", bytes);
  booking(&msg);
  if (send(ss_sock, &msg, sizeof(message_t), 0) < 0)
  {
    perror_tx("send");
    return;
  }
  int sent = 0;
  while (sent < bytes)
  {
    int ret2 = 0;
    if (bytes - sent >= SIZE)
    {
      if (ret2 = fread(msg.data, sizeof(char), SIZE, file) < 0)
      { // Directly using fread
        perror_tx("Error reading file");
        goto ret;
      }
      booking(&msg);
      if (send(ss_sock, &msg, sizeof(message_t), 0) < 0)
      {
        perror_tx("send");
        return;
      }
      sent += SIZE;
    } else {
      memset(msg.data, 0, SIZE);
      if (ret2 = fread(msg.data, sizeof(char), SIZE, file) < 0)
      { // Directly using fread
        perror_tx("Error reading file");
        goto ret;
      }
      booking(&msg);
      if (send(ss_sock, &msg, sizeof(message_t), 0) < 0)
      {
        perror_tx("send");
        return;
      }
      sent = bytes;
      break;
    }
  }
  msg.type = STOP;
  booking(&msg);
  if (send(ss_sock, &msg, sizeof(message_t), 0) < 0)
  {
    perror_tx("send");
    return;
  }
  logc(COMPLETION, "Sent %d bytes to storage server %s:%d", bytes, ip, port);

ret:
  fclose(file);
  free(path);
  free(localpath);
  free(ip);
  close(ss_sock);
}
