#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <stack>
#include <deque>
#include <iostream>
 
#include "../include/html_parser.h"
#include "../include/spider.h"

using namespace std;

char list_tag[24][6][20] = {
        {"a", "href", "ping", "#"},
        {"applet", "code", "archive", "codebase", "#"},
        {"area", "href", "#"},
        {"audio", "src", "#"},
        {"base", "href", "#"},
        {"blockquote", "cite", "#"},
        {"body", "background", "#"},
        {"button", "formaction", "#"},
        {"embed", "src", "#"},
        {"form", "action", "#"},
        {"frame", "longdesc", "src", "#"},
        {"head", "profile", "#"},
        {"html", "xmlns", "#"},
        {"iframe", "src", "#"},
        {"img", "longdesc", "src", "srcset", "#"},
        {"input", "formaction", "src", "#"},
        {"ins", "cite", "#"},
        {"link", "href", "#"},
        {"object", "codebase data", "#"},
        {"q", "cite", "#"},
        {"script", "src", "#"},
        {"source", "src", "srcset", "#"},
        {"track", "src", "#"},
        {"video", "poster", "src", "#"}
};

char *concat_path(char *path_base, char *path_relat) {

        deque<string> st;

        char buff[50000];
        char temp[100];
        buff[0] = '\0';

        if (path_relat[0] != '/') {
                strcat(buff, path_base);

                if (buff[strlen(buff) - 1] != '/') {
                        strcat(buff, "/");
                }
        }
        strcat(buff, path_relat);

        if(buff[0] == '/')
                sscanf(buff, "%*c%s", buff);

        char barra;

        while (sscanf(buff, "%[^/]%c%s", temp, &barra, buff) == 1) {

                if (strcmp(temp, "..") == 0) {
                        st.pop_back();
                } else {
                        string t(temp);
                        st.push_back(t);
                }

                if (buff == NULL && barra == '/') {
                        st.back().append("/");
                }
        }
        char *resp = (char *) malloc(5000 * sizeof(char));

        resp[0] = '/';
        resp[1] = '\0';

        while (!st.empty()) {
                strcat(resp, st.front().c_str());
                if (!st.empty())
                        strcat(resp, "/");

                st.pop_front();
        }

        return resp;
}

pair<string, string> find_url_path_of_value(char *url, char *path, char *value) {

        char *new_url;
        char *new_path;

        if (strncmp(value, "//", 2) == 0) {
                new_url = value + 2;
        } else if (strncmp(value, "http://", 7) == 0) {
                new_url = value + 7;
        } else if (strncmp(value, "https://", 8) == 0) {
                new_url = value + 8;
        } else {
                //Relative URL
                
                new_url = url;
                new_path = concat_path(path, value);

                string u(new_url), p(new_path);
                pair<string, string> resp;
                resp.first = u;
                resp.second = p;

                return resp;
        }

        size_t size_url, size_path;

        new_path = spider_separate_url_path(new_url, &size_url, &size_path);

        string u(new_url), p(new_path);
        pair<string, string> resp;
        resp.first = u;
        resp.second = p;

        return resp;
        
}

vector<pair<string, string> > get_ref_url(char *data, char *url, char *path) {

        FILE *fp = fopen("html_parser.temp", "w");
        fprintf(fp, "%s", data);
        fclose(fp);
        fp = fopen("html_parser.temp", "r");

        vector< pair<string, string> > urls;

        string s_url(url), s_path(path);

        stack<string> tags;

        bool close = false, is_script = false;

        char symbol;
        char tag[25], full_attr[10000];

        while(fscanf(fp, "%*[^<]%c %c", &symbol, &tag[0]) == 2) {

                if (symbol == '<') {
                        if (tag[0] == '/') {
                                close = true;
                                fscanf(fp, " %[^> \t\n\r]", tag); //name of tag
                        } else {
                                close = false;
                                fscanf(fp, " %[^> \t\n\r]", tag + 1); // tag[0] + tag + 1 = name of tag
                        }

                        printf("\tTAG: %s%s\n", close ? "/" : "", tag);

                        if (is_script == true && !(close == true && strcmp(tag, "script") == 0) )
                                continue; //Is not a real tag, is part of a Javascript

                        if (close == true) {
                                printf("\n\tClose: %s\n", tags.top().c_str());
                                tags.pop();
                        } else {
                                string temp(tag);
                                tags.push(temp);
                                printf("\n\tEnter: %s\n", tags.top().c_str());
                        }

                        char temp = '\0';
                        char attr[25], value[10000];

                        while (fscanf(fp, " %[^=> \t\n\r]%c", attr, &temp) == 2) {

                                printf("\t Attr: %s\tTemp: %c\n", attr, temp);

                                if (temp == '=')
                                        fscanf(fp, "\"%[^\"]\"", value);
                                else if (temp == '>')
                                        break;
                                else
                                        continue;

                                printf("\t Attr: %s\tValue: %s\n", attr, value);

                                if (value[0] == '\"') {
                                        sscanf(value + 1, "%[^\"]", value); //find string delimited by beginning '\"' and ending '\"'
                                        printf("\tNew value: %s\n", value);
                                }

                                for (int i = 0; i < 24; i++) {

                                        if (strcmp(list_tag[i][0], tag) != 0)
                                                continue;

                                        printf("\tTAG MATCH: %s\n", tag);

                                        for (int j = 1; list_tag[i][j][0] != '#'; j++) {

                                               printf("\t%d %s\n", j, list_tag[i][j]);
                                               if (strcmp(list_tag[i][j], attr) != 0)
                                                       continue;

                                               printf("\tATTR MATCH: %s\n", attr);

                                               pair<string, string> new_ref = find_url_path_of_value(url, path, value);

                                               cout << "\tURL: " << new_ref.first << "\tPATH: " << new_ref.second << "\n";

                                               urls.push_back(new_ref);

                                               break;
                                        }
                                }
                        }
                        if (strcmp(tag, "script") == 0) {
                                is_script = !close;
                        }

                        fscanf(fp, ">");
                } else {
                        printf("Unkown error has occurred\n");
                }
        }

        fclose(fp);

        return urls;
}
