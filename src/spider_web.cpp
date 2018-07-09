#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#include "../include/spider_web.h"

using namespace std;

int add_vis(struct web *wb, struct node *nd, char *name, int v) {
        if (nd == NULL || wb == NULL || name == NULL)
                return 1;

        if (nd->num_vis == 0) {
                nd->num_vis++;
                nd->vis = (pair<struct node *, int> *) malloc(nd->num_vis * sizeof(pair<struct node *, int>));

                if (nd->vis == NULL) {
                        nd->num_vis = 0;
                        return -1;
                }

                struct node *new_nd = create_node(wb, name);

                if (new_nd == NULL) {
                        nd->num_vis--;
                        free(nd->vis);
                        return -1;
                }

                nd->vis[nd->num_vis - 1].first = new_nd;
                nd->vis[nd->num_vis - 1].second = v;
                return 0;
        }

        for (int i = 0; i < nd->num_vis; i++)
                if (strcmp(nd->vis[i].first->name, name) == 0)
                        return 0;

        nd->num_vis++;
        nd->vis = (pair<struct node *, int> *) realloc(nd->vis, nd->num_vis * sizeof(pair<struct node *, int>));

        if (nd->vis == NULL) {
                nd->num_vis = 0;
                return -1;
        }

        struct node *new_nd = create_node(wb, name);

        if (new_nd == NULL) {
                nd->num_vis--;
                free(nd->vis);
                return -1;
        }

        nd->vis[nd->num_vis - 1].first = new_nd;
        nd->vis[nd->num_vis - 1].second = v;
        return 0;
}

struct node *create_node(struct web *wb, char *name) {

        if (wb == NULL || name == NULL)
                return NULL;

        struct node *new_nd;
        
        new_nd = obtain_node(wb, name);

        if (new_nd != NULL)
                return new_nd;

        new_nd = (struct node *) malloc(sizeof(struct node));

        if (new_nd == NULL)
                return NULL;

        new_nd->name = (char *) malloc(sizeof(char) * (strlen(name) + 1));

        if (new_nd->name == NULL) {
                free(new_nd);
                return NULL;
        }

        strcpy(new_nd->name, name);

        new_nd->num_vis = 0;
        new_nd->vis = NULL;

        if (add_node(wb, new_nd) != 0) {
                free(new_nd->name);
                free(new_nd);

                return NULL;
        }

        return new_nd;
}

struct node *obtain_node(struct web *wb, char *name) {
        if (wb == NULL || name == NULL)
                return NULL;

        for (int i = 0; i < wb->num_nodes; i++) {
                if (strcmp(wb->all_nodes[i]->name, name) == 0)
                        return wb->all_nodes[i];
        }
        return NULL;
}

int add_node(struct web *wb, struct node *nd) {
        if (wb == NULL || nd == NULL)
                return 1;

        if (obtain_node(wb, nd->name) != 0) {
                return 1;
        }

        if (wb->num_nodes == 0) {
                wb->all_nodes = (struct node **) malloc(sizeof(struct node *) * (wb->num_nodes + 1));
        } else {
                wb->all_nodes = (struct node **) realloc(wb->all_nodes, sizeof(struct node *) * (wb->num_nodes + 1));
        }
        if (wb->all_nodes == NULL) {
                //TODO
                return -1;
        }
        wb->num_nodes++;
        wb->all_nodes[wb->num_nodes - 1] = nd;

        if (wb->num_nodes == 1) {
                wb->central_node = nd;
        }

        return 0;
}

struct web *create_web(char *url, char *name) {
        struct web *wb = (struct web *) malloc(sizeof(struct web));

        if (wb == NULL) {
                return NULL;
        }
        wb->url = (char *) malloc(sizeof(char) * (strlen(url) + 1));
        if (wb->url == NULL) {
                free(wb);
                return NULL;
        }

        strcpy(wb->url, url);
        wb->central_node = NULL;
        wb->num_border_nodes = 0;
        wb->border_nodes = NULL;
        wb->num_nodes = 0;
        wb->all_nodes = NULL;

        struct node *nd = create_node(wb, name);

        if (nd == NULL) {
                free(wb->url);
                free(wb);

                return NULL;
        }

        if (wb->num_border_nodes != 0) {
                free(nd);
                free(wb->url);
                free(wb);

                return NULL;
        }

        wb->num_border_nodes++;

        wb->border_nodes = (struct node **) malloc(sizeof(struct node *) * wb->num_border_nodes);

        if (wb->border_nodes == NULL) {
                free(nd);
                free(wb->url);
                free(wb);

                return NULL;
        }

        wb->border_nodes[0] = nd;

        return wb;
}

int expand_web(struct web *wb) {
       //TODO

       return 0;
}
