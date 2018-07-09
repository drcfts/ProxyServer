#ifndef HTML_PARSER
#define HTML_PARSER

#include <string>
#include <vector>

using namespace std;

vector<pair<string, string> > get_ref_url(char *data, char *url, char *path);

#endif
