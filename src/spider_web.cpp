#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <vector>
#include <queue>

#include "../include/html_parser.h"
#include "../include/spider_web.h"
#include "../include/spider.h"

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

struct web *create_web(char *url) {

        char *path;
        size_t size_url, size_path;

        path = spider_separate_url_path(url, &size_url, &size_path);

        return create_web(url, path);
}

int expand_web(struct web *wb, int opt) {

        struct node *temp_border[5000];
        int temp_border_end = 0;

#if DEBUG_SPIDER_WEB
        printf("Iniciate expansion\n");
#endif

        for (int i = 0; i < wb->num_border_nodes; i++) {
                char *path = wb->border_nodes[i]->name;


                char *temp_url = (char *) malloc(sizeof(char) * (strlen(wb->url) + strlen(path) + 3));

                if (temp_url == NULL)
                        continue;

                temp_url[0] = '\0';
                strcat(temp_url, wb->url);
                strcat(temp_url, path);
                
#if DEBUG_SPIDER_WEB
                printf("\t Border: %s\n", temp_url);
#endif

                vector<pair<string, string> > vis = get_ref_url(temp_url, opt);

                for (auto it : vis) {
                        if (strcmp(it.first.c_str(), wb->url) != 0)
                                continue;

                        struct node *nd = obtain_node(wb, (char *) it.second.c_str());

                        if (nd != NULL) {
                                add_vis(wb, wb->border_nodes[i], (char *) it.second.c_str(), -1);
                        } else {
                                add_vis(wb, wb->border_nodes[i], (char *) it.second.c_str(), 1);

                                temp_border[temp_border_end++] = obtain_node(wb, (char *) it.second.c_str());
                        }
                }
        }
        wb->border_nodes = (struct node **) realloc(wb->border_nodes, sizeof(struct node *) * temp_border_end);
        wb->num_border_nodes = temp_border_end;

        for (int i = 0; i < wb->num_border_nodes; i++) {
                wb->border_nodes[i] = temp_border[i];
        }

#if DEBUG_SPIDER_WEB
        printf("Finish expansion\n");
#endif

        return 0;
}

map<string, int> tree_map;
vector< vector<pair<int, string> > > tree;
int n = 0;

int marc[100000];
int d[100000];

/*
void bfs(int a) {
        marc[0] = 1;

        printf("/\n");

        q.push(0);

        while (!q.empty()) {
                int atual = q.front();
                q.pop();

                //printf("Visiting %d\n", atual);

                for (pair<int, string> next : tree[atual]) {

                        //printf("Next %d\n", next.first);

                        if (!marc[next.first]) {
                                marc[next.first] = 1;
                                d[next.first] = d[atual] + 1;

                                for (int i = 0; i < d[next.first]; i++)
                                        printf("\t");
                                printf("%s\n", next.second.c_str());

                                q.push(next.first);
                        }
                }
        }
}
*/

void dfs(int a) {

        if (a == 0) {
                d[0] = 0;
                marc[0] = 1;

                printf("/\n");
        }

        for (pair<int, string> v : tree[a]) {

                if (!marc[v.first]) {
                        marc[v.first] = 1;
                        d[v.first] = d[a] + 1;

                        for (int i = 0; i < d[v.first]; i++)
                                printf("\t");
                        printf("%s\n", v.second.c_str());

                        dfs(v.first);
                }
        }
}

void add_tree(char *path) {

        char buff[5000], all_buff[50000];
        buff[0] = '\0';
        all_buff[0] = '\0';

        int parent = 0;

        char *path_x = (char *) malloc(sizeof(char) * (strlen(path) + 3));
                
        strcpy(path_x, path);

        if (path_x[0] == '/')
                path_x = path_x + 1;

        int ret;

        //printf("\tPath to be added: %s\n", path_x);

        while ((ret = sscanf(path_x, "%[^/]/%s", buff, path_x)) > 0) {

                strcat(all_buff, "/");
                strcat(all_buff, buff);

                //printf("\tCurr path: %s\n", buff);
                //printf("\tFull path: %s\n", all_buff);

                string dir(all_buff);

                auto it = tree_map.find(dir);

                if (it == tree_map.end()) {

                        //printf("\tit == end\n");

                        tree_map.emplace(dir, n);

                        string name(all_buff);
                        if (ret != 1)
                                name.append("/");

                        tree[parent].emplace_back(n, name);
                        //printf("\tAdded %d %d\n", parent, n);

                        parent = n;

                        n++;
                        tree.resize(n);
                } else {
                        parent = it->second;
                }

                if (ret != 2)
                        break;
        }
}

void print(struct web *wb) {

        tree_map.clear();
        n = 0;

        string root("/");

        tree_map.emplace(root, 0);
        n++;

        tree.resize(n);

        for (int i = 0; i < wb->num_nodes; i++) {
                char *path = wb->all_nodes[i]->name;

                //printf("Adding %s to tree\n", path);

                if (strcmp(path, "/") == 0)
                        continue;

                add_tree(path);
        }
        //printf("All nodes added to tree\n");

        printf("\t\tSPIDER\n###################################\n");

        for (int i = 0; i < n; i++) {
                d[i] = 1000000000;
                marc[i] = 0;
        }
        
        d[0] = 0;
        marc[0] = 0;

        dfs(0);

        printf("###################################\n");

}

struct web *dump(char *url) {
        struct web *wb = create_web(url);

        while (wb->border_nodes != 0) {
                expand_web(wb, 1);
        }

        return wb;
}

struct web *spider(char *url) {
        struct web *wb = create_web(url);

        while (wb->border_nodes != 0) {
                print(wb);

                expand_web(wb, 0);
        }

        return wb;
}
