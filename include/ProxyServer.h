#ifndef HTTP_PROXY
#define HTTP_PROXY

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include "ProxyParser.h"

class ProxyServer{
public:
  ProxyServer(int port);
  void ProxyRequest(int client_fd, struct sockaddr_in clientAddr, socklen_t clientAddrSize);

  //Descritor de socket vindo da função socket
  int mSocketDescriptor;
  int getProxyDescriptor(){
    return mSocketDescriptor;
  }

private:

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
