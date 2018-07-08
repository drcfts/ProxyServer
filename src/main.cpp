#include <iostream>
#include <bits/stdc++.h>
#include <unistd.h> //fork()

#include "../include/ProxyParser.h"
#include "../include/ProxyServer.h"

using namespace std;

int main(int argc, char const *argv[]) {
  int num_port = 0;

  //Checagem do numero de argumentos
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

  //Criacao do objeto com o numero de porta
  ProxyServer httpproxy(num_port);

  //Estruturas para aceitar conexoes da fila
  int proxy_fd = httpproxy.getProxyDescriptor();
  struct sockaddr_in clientAddr;
  socklen_t clientAddrSize = sizeof(clientAddr);

  //Loop para executar
  while(1){

    int client_fd = accept(proxy_fd, (struct sockaddr *)&clientAddr, &clientAddrSize);

    //Fork duplica o processo: para o pai, retorna o id do filho; para o filho, retorna 0
    int PID = fork();

    //Se for filho, cria uma conexao
    if(PID == 0){
      httpproxy.ProxyRequest(client_fd, clientAddr, clientAddrSize);
    }
    //Se for pai, apenas fecha o descriptor no processo pai
    else{
      close(client_fd);
    } //else
  } //while(true)

  return 0;
}
