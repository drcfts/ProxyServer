#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#include "../include/spider_tree.h"

using namespace std;

map <string, struct tree *> url_spider_tree;

int add_url(char *url) {

        string str_url(url);

        auto it = url_spider_tree.find(url);

        if (it != url_spider_tree.end()) {
                return 1;
        }

       struct tree *new_url = create_tree(); 

       if (new_url == NULL)
               return -1;

       url_spider_tree.insert(make_pair(url, new_url));

       return 0;
}

int add_dir(struct tree *root, char *path) {

        char *dir_atual;
        char a;

        if (strcmp(path, "") == 0)
                return 0;

        sscanf(path, "%m[^/]%c", &dir_atual, &a);

        if (a == '/') {
                strcat(dir_atual, "/");
        }

        if (strcmp(root->dir, dir_atual) != 0)
                return -1;

        char *dir_next;

        sscanf(path, "%*[^/]/%s", path);
        sscanf(path, "%m[^/]%c", &dir_next, &a);

        if (a == '/') {
                strcat(dir_next, "/");
        }

        if (root->sub_dir == NULL) {
                root->sub_dir_num++;

                root->sub_dir = (struct tree **) malloc(sizeof(struct tree *) * root->sub_dir_num);

                struct tree *sub_dir = create_tree(dir_next);

                root->sub_dir[0] = sub_dir;

                return add_dir(sub_dir, path);
        }

        for (int i = 0; i < root->sub_dir_num; i++) {
                if (strcmp(dir_next, root->sub_dir[i]->dir) == 0) {
                        return add_dir(root->sub_dir[i], path);
                }
        }
        root->sub_dir_num++;

        root->sub_dir = (struct tree **) realloc(root->sub_dir, sizeof(struct tree *) * root->sub_dir_num);

        struct tree *sub_dir = create_tree(dir_next);

        root->sub_dir[root->sub_dir_num - 1] = sub_dir;

        return add_dir(sub_dir, path);
}

int add_dir(char *url, char *path) {
        string str_url(url);

        auto it = url_spider_tree.find(url);

        if (it == url_spider_tree.end()) {
                return 1;
        }

        struct tree *root = it->second;

        return add_dir(root, path);
}

struct tree *create_tree() {
        struct tree *new_tree = (struct tree *) malloc(sizeof(struct tree));

        const char *a = "/";

        if (new_tree == NULL)
                return NULL;

        new_tree->dir = (char *) malloc(sizeof(char) * strlen(a));
        strcpy(new_tree->dir, a);
        new_tree->sub_dir_num = 0;
        new_tree->sub_dir = NULL;

        return new_tree;
}

struct tree *create_tree(char *dir) {
        struct tree *new_tree = (struct tree *) malloc(sizeof(struct tree));

        if (new_tree == NULL)
                return NULL;

        new_tree->dir = (char *) malloc(sizeof(char) * strlen(dir));
        strcpy(new_tree->dir, dir);
        new_tree->sub_dir_num = 0;
        new_tree->sub_dir = NULL;

        return new_tree;
}
