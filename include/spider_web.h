#ifndef HTTP_SPIDER_TREE
#define HTTP_SPIDER_TREE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#define DEBUG_SPIDER_WEB 1

using namespace std;

struct node {
        char *name;

        int num_vis;
        pair<struct node *, int> *vis;
};

struct web {

        char *url;
        
        struct node *central_node;
        int num_border_nodes;
        struct node **border_nodes;
        int num_nodes;
        struct node **all_nodes;

};

int add_vis(struct web *wb, struct node *nd, char *vis, int v);

struct node *create_node(struct web *wb, char *name);

struct node *obtain_node(struct web *wb, char *name);

int add_node(struct web *wb, struct node *nd);



struct web *create_web(char *url, char *name);

struct web *create_web(char *url);

int expand_web(struct web *wb, int opt);

void print(struct web *wb);

struct web *spider(char *url);

struct web *dump(char *url);

#endif
