#ifndef HTTP_PROXY
#define HTTP_PROXY

#include "ProxyParser.h"

class ProxyServer{
public:
  ProxyServer(int port);
  void ProxyRequest();

private:
  //Descritor de socket vindo da função socket
  int mSocketDescriptor;
  //Envia de volta pro cliente
  void ProxyBackClient(int client_fd, int remote_socket);
  //Envia a requisicao pro remote
  void SendRequestRemote(const char *req_string, int remote_socket, int buff_length);
  //Cria socket remoto
  int CreateRemoteSocket(char* remote_addr, char* port);
  //Cria socket servidor, começa a ouvir request na porta indicada por port
  void CreateServerSocket(int port);
  //Transforma objeto pra string
  char* RequestToString(RequestFields *req);


};

#endif
