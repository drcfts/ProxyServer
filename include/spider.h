#ifndef HTTP_SPIDER
#define HTTP_SPIDER

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SPIDER_MSG_SIZE 500

#define DEBUG_SPIDER 1

char *spider_get_response(char *request);

char *spider_get_data(char *url);

char *spider_separate_url_path(char *url, size_t *url_size, size_t *path_size);

#endif
