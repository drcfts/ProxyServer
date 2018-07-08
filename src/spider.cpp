#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#include <sys/types.h>
#include <sys/socket.h>
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

                //printf("\t%d %d %d\n", *msg_size, end_msg, bytes);
                //printf("\tBytes: %d\n%s\n", bytes, msg);
                *msg_size += bytes;
                end_msg += bytes;

                //printf("\tNew size: %d\n", *msg_size);
                msg = (char *) realloc(msg, *msg_size * sizeof(char));
                //printf("\tRealloced\n%s\n", msg);

                msg[end_msg] = '\0';
                if (msg == NULL) {
                        //printf("XXXXXXXXXXXXX\n");
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

        printf("\tUrl: %s\n\tPort Number: %s\n", url, port);

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

map<pair<string, string>, int> hostname_ip;

int spider_dns_get_ip(char *url, char *port) {

        string str_url(url);
        string str_port(port);

        pair<string, string> key = make_pair(str_url, str_port);

        auto it = hostname_ip.find(key);

        if(it != hostname_ip.end()) {
                printf("\tSocket (saved): %d\n", it->second);
                return it->second;
        }

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

        printf("\t Host address: %s\n", resp->ai_addr->sa_data);

        int sck_fd = socket(resp->ai_family, resp->ai_socktype, resp->ai_protocol);

        if (sck_fd == -1) {
                perror("Error in socket creation");
        }

        int ret = connect(sck_fd, resp->ai_addr, resp->ai_addrlen);

        if (ret == -1) {
                perror("Error in connection establishment");
        }

        printf("\tSocket: %d\n", sck_fd);

        hostname_ip.insert(make_pair(key, sck_fd));

        return sck_fd;
        
}

char *spider_replace_url_path(char *request, size_t request_size, char *path, size_t path_size) {

        char *request2 = (char *) malloc(request_size);

        char buff1[5000], buff2[5000];

        sscanf(request, "%s %s", buff1, buff2);

        sprintf(request2, "%s %s %s", buff1, path, request + strlen(buff1) + strlen(buff2) + 1);

        return request2;
}

void spider_get_response(int request_fd, struct sockaddr_in cliente_addr, socklen_t cliente_addr_size) {
        
        char *request;
        size_t request_size;

        printf("\tBegin Spider\n");

        request = spider_get_full_msg(request_fd, &request_size);

        printf("\tSpider Request:\n%s\n\tSpider Request End\n", request);

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

        printf("\tSocket Received: %d\n", response_fd);

        char *request2 = spider_replace_url_path(request, request_size, path, path_size);
        size_t request2_size = strlen(request2);

        printf("\tSpider Request-send:\n%s\n\tSpider Request-send End\n", request2);
        send(response_fd, request2, request2_size, 0);

        char *response;
        size_t response_size;

        response = spider_get_full_msg(response_fd, &response_size);

        printf("\tSpider Response:\n%s\n\tSpider Response End\n", response);
        send(request_fd, response, response_size, 0);
}
