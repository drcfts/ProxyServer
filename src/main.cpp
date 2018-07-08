#include <iostream>
#include <bits/stdc++.h>
#include "../include/ProxyParser.h"
#include "../include/ProxyServer.h"

using namespace std;

int main(int argc, char const *argv[]) {
  int num_port = 0;

  if(argc == 1){
    num_port = 8228;
  }
  else if(argc == 3){
    if(strcmp(argv[1], "-p")){
      cout << "Invalid -p argument!" << endl;
      exit(-1);
    }
    num_port = strtol(argv[2], NULL, 10);
    if(num_port <= 1023){
      cout << "Invalid port number!" << endl;
      exit(-2);
    }
  }
  //Numero de argumentos errado
  else{
    cout << "Invalid number of arguments!" << endl;
    exit(-3);
  }

  ProxyServer httpproxy(num_port);
  while(true){
    cout << "proxy request" << endl;
    httpproxy.ProxyRequest();
  }

  return 0;
}
