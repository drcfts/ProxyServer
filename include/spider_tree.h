#ifndef HTTP_SPIDER_TREE
#define HTTP_SPIDER_TREE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

using namespace std;

struct tree {
        char *dir;

        int sub_dir_num;
        struct tree **sub_dir;
};

int add_url(char *url);

int add_dir(char *url, char *path);

//int del_url(char *url);

//int del_dir(char *url, char *path);

struct tree *create_tree();

struct tree *create_tree(char *dir);

#endif
