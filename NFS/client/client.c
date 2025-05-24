#include "client.h"


int name_socket;
logfile_t *logfile;

int main(int argc, char const *argv[])
{
#ifdef LOG
  if (argc < 2)
  {
    fprintf(stderr, "usage: %s <logfile>\n", argv[0]);
    exit(1);
  }
  logfile = (logfile_t *)calloc(1, sizeof(logfile_t));
  strcpy(logfile->path, argv[1]);
  pthread_mutex_init(&(logfile->lock), NULL);

#else
  logfile = (logfile_t *)calloc(1, sizeof(logfile_t));
  logfile->path[0] = 0;
  pthread_mutex_init(&(logfile->lock), NULL);

#endif

  // signal_tx(SIGPIPE, SIG_IGN);
  for (int i = 1; i > 0;)
  {
    name_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (name_socket < 0)
    {
      printf("Unable to create socket\n");
      return -1;
    }
    NAME_IP = argv[1];
    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NAME_PORT);
    addr.sin_addr.s_addr = inet_addr(NAME_IP);

    if (connect(name_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
      perror("Unable to connect to Naming Server");
      return -1;
    }
    printf("[R] for Read\n");
    printf("[W] for Write\n");
    printf("[P] for Copy\n");
    printf("[D] for Delete\n");
    printf("[C] for Create\n");
    printf("[I] for Info\n");
    printf("[L] for List\n");
    printf("[Q] for Exit\n");
    printf("[S] for Stream\n");

    char c;
    scanf(" %c", &c);
    c = toupper(c);

    if (c == 'R')
    {
      request_read(name_socket);
    }
    else if (c == 'W')
    {
      request_write(name_socket);
    }
    else if (c == 'C')
    {
      request_create(name_socket);
    }
    else if (c == 'D')
    {
      request_delete(name_socket);
    }
    else if (c == 'P')
    {
      request_copy(name_socket);
    }
    else if (c == 'S')
    {
      request_stream(name_socket);
    }
    else if (c == 'L')
    {
      request_list(name_socket);
    }
    else if (c == 'I')
    {
      request_info(name_socket);
    }
    else if (c == 'Q')
    {
      goto ret_main;
    }
    else
    {
      printf("Invalid Request\n");
    }

    close(name_socket);
  }
  ret_main:
  printf("\n");
  close(name_socket);
  pthread_mutex_destroy(&(logfile->lock));
  free(logfile);
  return 0;
}
