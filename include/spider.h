#ifndef HTTP_SPIDER
#define HTTP_SPIDER

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SPIDER_MSG_SIZE 500

void spider_get_response(int cliente_fd, struct sockaddr_in cliente_addr, socklen_t cliente_addr_size);

#endif
