#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#include "../include/spider.h"

using namespace std;

char *spider_get_full_msg(int request_fd, size_t *msg_size) {
        char *msg;
        size_t end_msg = 0;
        *msg_size = SPIDER_MSG_SIZE;

        int bytes = 0;

        msg = (char *) malloc(*msg_size * sizeof(char));

        if (msg == NULL) {
                *msg_size = 0;
                return NULL;
        }

        msg[end_msg] = '\0';

        while ( (bytes = recv(request_fd, msg + end_msg, SPIDER_MSG_SIZE, 0)) > 0) {

                *msg_size += bytes;
                end_msg += bytes;

                msg = (char *) realloc(msg, *msg_size * sizeof(char));

                msg[end_msg] = '\0';
                if (msg == NULL) {
                        *msg_size = 0;
                        return NULL;
                }

                if (bytes < SPIDER_MSG_SIZE)
                        break;
        }
        return msg;
}

char *spider_get_url (char *msg, size_t *url_size) {
        char *url;

        sscanf(msg, "%*s %ms", &url);

        printf("\tOriginal Url: %s\n", url);

        if (strncmp(url, "http://", 7) == 0) { //If the 7 first characters are http://
                sscanf(url, "http://%s", url);
        } else if (strncmp(url, "https://", 8) == 0) { //If the 8 first characters are https://
                sscanf(url, "https://%s", url);
        }

        printf("\tNew Url: %s\n", url);

        *url_size = strlen(url);

        return url;
}

char *spider_separate_url_port(char *url, size_t *url_size, size_t *port_size) {

        char *port = (char *) malloc(10 * sizeof(char));

        port[0] = '\0';

        sscanf(url, "%*[^:]:%s", port);
        sscanf(url, "%[^:]", url);

        if (port[0] == '\0') {
                port[0] = '8';
                port[1] = '0';
                port[2] = '\0';
        }

#if DEBUG
#if DEBUG_SPIDER
        printf("\tUrl: %s\n\tPort Number: %s\n", url, port);
#endif
#endif

        *url_size = strlen(url);
        *port_size = strlen(port);

        return port;
}

char *spider_separate_url_path(char *url, size_t *url_size, size_t *path_size) {
        char *path;

        sscanf(url, "%*[^/]%ms", &path);
        sscanf(url, "%[^/]", url);

        *url_size = strlen(url);
        *path_size = strlen(path);

        return path;
}

int spider_dns_get_ip(char *url, char *port) {

        struct addrinfo hints;
        struct addrinfo *resp;

        hints.ai_flags = 0;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = 0;
        hints.ai_addrlen = 0;
        hints.ai_addr = NULL;
        hints.ai_canonname = NULL;
        hints.ai_next = NULL;

        int s = getaddrinfo(url, port, &hints, &resp);

        if (s != 0) {
                fprintf(stderr, "Error in getaddrinfo: %s\n", gai_strerror(s));
                return -1;
        }

#if DEBUG
#if DEBUG_SPIDER
        printf("\t Host address: %s\n", resp->ai_addr->sa_data);
#endif
#endif

        int sck_fd = socket(resp->ai_family, resp->ai_socktype, resp->ai_protocol);

        if (sck_fd == -1) {
                perror("Error in socket creation");
        }

        int ret = connect(sck_fd, resp->ai_addr, resp->ai_addrlen);

        if (ret == -1) {
                perror("Error in connection establishment");
        }

#if DEBUG
#if DEBUG_SPIDER
        printf("\tSocket: %d\n", sck_fd);
#endif
#endif

        return sck_fd;
        
}

char *spider_replace_url_path(char *request, size_t request_size, char *path, size_t path_size) {

        char *request2 = (char *) malloc(request_size);

        char buff1[5000], buff2[5000];

        sscanf(request, "%s %s", buff1, buff2);

        sprintf(request2, "%s %s %s", buff1, path, request + strlen(buff1) + strlen(buff2) + 1);

        return request2;
}

char *spider_get_response(char *request) {
        
        size_t request_size;

#if DEBUG
#if DEBUG_SPIDER
        printf("\tBegin Spider\n");
        printf("\tSpider Request:\n%s\n\tSpider Request End\n", request);
#endif
#endif

        char *url;
        size_t url_size;

        url = spider_get_url(request, &url_size);

        char *str_port;
        size_t str_port_size;

        str_port = spider_separate_url_port(url, &url_size, &str_port_size);

        char *path;
        size_t path_size;

        path = spider_separate_url_path(url, &url_size, &path_size);

        int response_fd = spider_dns_get_ip(url, str_port);

/*
        char *request2 = spider_replace_url_path(request, request_size, path, path_size);
        size_t request2_size = strlen(request2);

#if DEBUG
#if DEBUG_SPIDER
        printf("\tSpider Request-send:\n%s\n\tSpider Request-send End\n", request2);
#endif
#endif
        send(response_fd, request2, request2_size, 0);
*/
        send(response_fd, request, request_size, 0);

        char *response;
        size_t response_size;

        response = spider_get_full_msg(response_fd, &response_size);

#if DEBUG
#if DEBUG_SPIDER
        printf("\tSpider Response:\n%s\n\tSpider Response End\n", response);
#endif
#endif
        //send(request_fd, response, response_size, 0);
        

        //CLOSE CONNECTIONS
        
        close(response_fd);

        return response;
}
