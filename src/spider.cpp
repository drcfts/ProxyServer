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

        msg = (char *) malloc((*msg_size + 1) * sizeof(char));

        if (msg == NULL) {
                *msg_size = 0;
                return NULL;
        }

        msg[end_msg] = '\0';

        while ( (bytes = recv(request_fd, msg + end_msg, SPIDER_MSG_SIZE, 0)) > 0) {

                //printf("RECV:\n%s\n\t-------------\n", msg);

                *msg_size += bytes;
                end_msg += bytes;

                msg[end_msg] = '\0';

                //printf("%p, %d\n", msg, (*msg_size + 1) * sizeof(char));
                msg = (char *) realloc(msg, (*msg_size + 1) * sizeof(char));
                //printf("DONE\n");

                if (msg == NULL) {
                        *msg_size = 0;
                        return NULL;
                }

                if (bytes < SPIDER_MSG_SIZE) {
                        printf("BYTES: %d\t TOTAL: %lu\n", bytes, end_msg);
//                        break;
                }
        }
        return msg;
}

char *spider_get_url (char *msg, size_t *url_size) {
        char *url;

        sscanf(msg, "%*s %ms", &url);

#if DEBUG_SPIDER
        printf("\tOriginal Url: %s\n", url);
#endif

        if (strncmp(url, "http://", 7) == 0) { //If the 7 first characters are http://
                sscanf(url, "http://%s", url);
        } else if (strncmp(url, "https://", 8) == 0) { //If the 8 first characters are https://
                sscanf(url, "https://%s", url);
        }

#if DEBUG_SPIDER
        printf("\tNew Url: %s\n", url);
#endif

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

#if DEBUG_SPIDER
        printf("\tUrl: %s\n\tPort Number: %s\n", url, port);
#endif

        *url_size = strlen(url);
        *port_size = strlen(port);

        return port;
}

char *spider_separate_url_path(char *url, size_t *url_size, size_t *path_size) {
        char *path;

        sscanf(url, "%*[^/]%ms", &path);

        if (path == NULL) {
                path = (char *) malloc(sizeof(char) * 2);
                if (path == NULL)
                        return NULL;

                path[0] = '/';
                path[1] = '\0';
        }

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

#if DEBUG_SPIDER
        printf("\t Host address: %s\n", resp->ai_addr->sa_data);
#endif

        int sck_fd = socket(resp->ai_family, resp->ai_socktype, resp->ai_protocol);

        struct timeval timeout;

        timeout.tv_sec = 4;
        timeout.tv_usec = 0;

        setsockopt(sck_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout) );

        if (sck_fd == -1) {
                perror("Error in socket creation");
        }

        int ret = connect(sck_fd, resp->ai_addr, resp->ai_addrlen);

        if (ret == -1) {
                perror("Error in connection establishment");
        }

#if DEBUG_SPIDER
        printf("\tSocket: %d\n", sck_fd);
#endif

        free(resp);

        return sck_fd;
        
}

char *spider_replace_url_path(char *request, size_t request_size, char *path, size_t path_size) {

        char *request2 = (char *) malloc((request_size + 2) * sizeof(char)); // + 1 para o '\0'

        char buff1[5000], buff2[5000];

        sscanf(request, "%s %s", buff1, buff2);

        sprintf(request2, "%s %s %s", buff1, path, request + strlen(buff1) + strlen(buff2) + 2);

        return request2;
}

char *spider_get_response(char *request) {
        
        size_t request_size;

#if DEBUG_SPIDER
        printf("\tBegin Spider\n");
        printf("\tSpider Request:\n%s\n\tSpider Request End\n", request);
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

        request_size = strlen(request);

        char *request2 = spider_replace_url_path(request, request_size, path, path_size);
        size_t request2_size = strlen(request2);

#if DEBUG_SPIDER
        printf("\tSpider Request-send:\n%s\n\tSpider Request-send End\n", request2);
#endif
        send(response_fd, request2, request2_size, 0);

        //send(response_fd, request, request_size, 0);

        char *response;
        size_t response_size;

        response = spider_get_full_msg(response_fd, &response_size);

#if DEBUG_SPIDER
        printf("\tSpider Response:\n%s\n\tSpider Response End\n", response);
#endif
        //send(request_fd, response, response_size, 0);
        

        //CLOSE CONNECTIONS
        
        close(response_fd);

        return response;
}

char *spider_create_request(char *url) {

        size_t url_size = strlen(url);

        char *buff = (char *) malloc (sizeof(char) * (50 + 2 * url_size)); // 1 url_size da url; 1 url_size do host;
                                                                           //25 bytes de strings constantes e 25 bytes de folga
        if (buff == NULL)
                return NULL;
        buff[0] = '\0';

        strcat(buff, "GET ");
        strcat(buff, url);
        strcat(buff, " HTTP/1.1\r\nHost: ");


        char *str_port;
        size_t str_port_size;

        str_port = spider_separate_url_port(url, &url_size, &str_port_size);

        char *path;
        size_t path_size;

        path = spider_separate_url_path(url, &url_size, &path_size);

        strcat(buff, url); //Tira a porta e o caminho para obter o nome do Host
        strcat(buff, "\r\n\r\n");

        free(path);
        free(str_port);

        return buff;
}

char *spider_get_data(char *url) {
        char *request = spider_create_request(url);

        char *response = spider_get_response(request);
        size_t response_size = strlen(response);

        char *resp = (char *) malloc(sizeof(char) * (response_size + 1));

        if (resp == NULL) {
                return NULL;
        }

        for (int i = 0; i < (int) response_size; i++) {

                //if (response[i] == '\n' || response[i] == '\r')
                //        printf("%c\n", response[i] == '\n' ? 'n' : 'r');

                if (strncmp(response + i, "\r\n\r\n", 4) == 0) {
                        strcpy(resp, response + i + 4); //Tudo que vier depois de "\r\n\r\n"
                        return resp;
                }
        }
        return NULL;
}
