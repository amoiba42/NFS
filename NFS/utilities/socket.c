#include "socket.h"

extern logfile_t* logfile;

// for txn   : req = NULL, use_tpx = 0
// for tpx  : req, use_tpx = 1

ssize_t send_txn(int sockfd, void *buf, size_t len, int flags, request_t *req, int use_tpx)
{
    ssize_t total_len = len; 
    ssize_t ret;
    while (len > 0)
    {
        ret = send(sockfd, buf, len, flags);
        if (ret < 0)
        {
            if (use_tpx && req != NULL)
            {
                perror_tpx(req, "send");
            }
            else
            {
                perror_tx("send");
            }
            return -1;
        }

        buf += ret;
        len -= ret;
    }
    return total_len;
}

ssize_t recv_txn(int sockfd, void *buf, size_t len, int flags, request_t *req, int use_tpx)
{
    ssize_t total_len = len;
    ssize_t ret;
    while (len > 0)
    {
        ret = recv(sockfd, buf, len, flags);
        if (ret < 0)
        {
            if (use_tpx && req != NULL)
            {
                perror_tpx(req, "recv");
            }
            else
            {
                perror_tx("recv");
            }
            return -1;
        }

        buf += ret;
        len -= ret;
    }
    return total_len;
}

int accept_txn(int sockfd, struct sockaddr *addr, socklen_t *addrlen, request_t *req, int use_tpx)
{
    int newsock = accept(sockfd, addr, addrlen);
    if (newsock < 0)
    {
        if (use_tpx && req != NULL)
        {
            perror_tpx(req, "accept");
        }
        else
        {
            perror_tx("accept");
        }
        return -1;
    }
    return newsock;
}

int listen_txn(int sockfd, int backlog, request_t *req, int use_tpx)
{
    int ret = listen(sockfd, backlog);
    if (ret < 0)
    {
        if (use_tpx && req != NULL)
        {
            perror_tpx(req, "listen");
        }
        else
        {
            perror_tx("listen");
        }
        return -1;
    }
    return ret;
}

int socket_txn(int domain, int type, int protocol, request_t *req, int use_tpx)
{
    int sockfd = socket(domain, type, protocol);
    if (sockfd < 0)
    {
        if (use_tpx && req != NULL)
        {
            perror_tpx(req, "socket");
        }
        else
        {
            perror_tx("socket");
        }
        return -1;
    }
    return sockfd;
}

int setsocket_txn(int sockfd, int level, int optname, const void *optval, socklen_t optlen, request_t *req, int use_tpx)
{
    int ret = setsockopt(sockfd, level, optname, optval, optlen);
    if (ret < 0)
    {
        if (use_tpx && req != NULL)
        {
            perror_tpx(req, "setsockopt");
        }
        else
        {
            perror_tx("setsockopt");
        }
        return -1;
    }
    return ret;
}

int bind_txn(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret;

    if (pthread_mutex_lock(&(logfile->lock)) != 0)
    {
        perror_tx("pthread_mutex_lock");
        return -1;
    }

    ret = bind(sockfd, addr, addrlen);
    if (ret < 0)
    {
        perror_tx("bind");
        pthread_mutex_unlock(&(logfile->lock));
        return -1;
    }

    if (pthread_mutex_unlock(&(logfile->lock)) != 0)
    {
        perror_tx("pthread_mutex_unlock failed");
        return -1;
    }
    return ret;
}

int connect_retry(int sockfd, const struct sockaddr* addr, socklen_t addrlen, int* timeout)
{
  while (1) {
    if (connect(sockfd, addr, addrlen) == 0) {
      return 0;  
    }
    
    perror_tx("connect");
    pthread_mutex_lock_tx(&(logfile->lock));
    fprintf(stderr, RED_NORMAL "Connect Error: Retrying after %d seconds..." RESET "\n", RETRY_DIFF);
    pthread_mutex_unlock_tx(&(logfile->lock));
    
    sleep(RETRY_DIFF);

    if (timeout != NULL) {
      *timeout -= RETRY_DIFF;
      if (*timeout <= 0) {
        return 1;  
      }
    }
  }
}

int connect_t(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
  return connect_retry(sockfd, addr, addrlen, NULL);
}

int connect_after(int sockfd, const struct sockaddr* addr, socklen_t addrlen, int timeout)
{
  return connect_retry(sockfd, addr, addrlen, &timeout);
}


