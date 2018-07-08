#include "../include/ProxyServer.h"
#include "../include/ProxyParser.h"

#include <iostream>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h> // close()

#include <netdb.h> // getaddrinfo

using namespace std;

// Começa a aceitar requests
void ProxyServer::CreateServerSocket(int port){
  mSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

  const int optval = 1;
  //Deixa outros sockets a fazer bind nessa porta
  setsockopt(mSocketDescriptor, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

  struct sockaddr_in serverAddr;
  bzero(&serverAddr, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  //Colocar proprio endereco
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddr.sin_port = htons(port);
  struct sockaddr *sa = (struct sockaddr *) &serverAddr;

  ::bind(mSocketDescriptor, sa, sizeof(serverAddr));

  const size_t MaxQueuedRequests = 64;
  listen(mSocketDescriptor, MaxQueuedRequests);
  cout << "Listening on port: " << port << endl;
}

//Construtor
ProxyServer::ProxyServer(int port){
  CreateServerSocket(port);
}

/* Métodos públicos */


void ProxyServer::ProxyRequest(){
  struct sockaddr_in clientAddr;
  socklen_t clientAddrSize = sizeof(clientAddr);

  //Escrever a conexao do cliente a vir pro sockaddr
  int client_fd = accept(mSocketDescriptor, (struct sockaddr *) &clientAddr, &clientAddrSize);

  //Pega enderco do cliente e transforma em in_addr
  const char *clientIPAddress = inet_ntoa(clientAddr.sin_addr);
  uint16_t clientPort = ntohs(clientAddr.sin_port);
  cout << "Server got connection from client:" << clientIPAddress << clientPort << endl;

  //encaminhamento do request
  int MAX_BUFFER_SIZE = 5000;
  char buf[MAX_BUFFER_SIZE];
  char *request_message = (char *) malloc(MAX_BUFFER_SIZE);
  request_message[0] = '\0';
  int total_received_bits = 0;

  //recebe n requests do cliente do strem pro buffer
  while(strstr(request_message, "\r\n\r\n") == NULL){
    //Retorna numero de bytes recebidos, armazena em buf
    int byte_recvd = recv(client_fd, buf, MAX_BUFFER_SIZE, 0);
    total_received_bits += byte_recvd;
    //Ultima posicao
    buf[byte_recvd] = '\0';

    if(total_received_bits > MAX_BUFFER_SIZE){
      MAX_BUFFER_SIZE *=2;
      request_message = (char *) realloc(request_message, MAX_BUFFER_SIZE);
    } //if
    // Vai recebendo de pouco em pouco a requisição e armazenando no buffer
    strcat(request_message, buf);
  } //while
  cout << "Request_Message: " << request_message << endl;

  RequestFields *req;
  req = RequestFields_create();

  //Faz o parser da msg recebida
  if(RequestFields_parse(req, request_message, strlen(request_message))<0){
    cout << " Request message format not supported" << endl;
  }
  else{
    if(req->port == NULL){
      req->port = (char *) "80";
    }
    char* req_string = RequestToString(req);
    cout << "req_string: " << req_string << endl;

    cout << "client host n port: " << req->host << req->port << endl;
    // Conexao ao socket remote_socket
    int remote_socket = CreateRemoteSocket(req->host, req->port);

    cout << "SendRequestRemote: " << remote_socket << " total received bits" << total_received_bits << endl;

    SendRequestRemote(req_string, remote_socket, total_received_bits);

    cout << "ProxyBackClient" << endl;
    //Manda de volta pro cliente
    ProxyBackClient(client_fd, remote_socket);
    //Dá free na estrutura criada
    RequestFields_destroy(req);
    close(client_fd);
    close(remote_socket);
  } //else

} //funcao

/* Métodos privados */

//Recebe resposta remota, manda pro cliente
//Cabeçalho já foi enviado e armazenado, agora envio do objeto
void ProxyServer::ProxyBackClient(int client_fd, int remote_socket){
  int MAX_BUFFER_SIZE = 5000;
  int buff_len;
  char received_buf[MAX_BUFFER_SIZE];

  while((buff_len = recv(remote_socket, received_buf, MAX_BUFFER_SIZE, 0)) > 0){
    cout << "received from remote: " << buff_len << endl;
    int totalsent = 0;
    int senteach;
    while(totalsent < buff_len){
      if((senteach = send(client_fd, (void *) (received_buf + totalsent), buff_len - totalsent, 0)) < 0){
        exit(1);
      }

      totalsent += senteach;
      cout << "sending back to client" << totalsent << endl;
      //Zera o buffer pra ler dnv
      memset(received_buf, 0, sizeof(received_buf));
    } //while interno
  } //while
}

//Cria o socket remoto para comunicacao, de acordo com a porta e o address especificado pela requisicao
int ProxyServer::CreateRemoteSocket(char *remote_addr, char *port){
  // Pegar resultados sobre nome de host e colocar em hints
  struct addrinfo hints, *servinfo;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  //Consultas DNS
  if(getaddrinfo(remote_addr, port, &hints, &servinfo) !=0){
    cout << "Error in server address format!\n" << endl;
  }

  int remote_socket;
  //Com os infos do servidor, cria o socket remoto
  if((remote_socket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol))<0){
    cout << "Error in creating socket to server!\n" << endl;
  }

  //Ponteiro de arquivo para o servidor conectado
  if(connect(remote_socket, servinfo->ai_addr, servinfo->ai_addrlen) < 0){
    cout << "Error in connecting to server!\n" << endl;
  }

  freeaddrinfo(servinfo);
  return remote_socket;

}

//Envio do request para o servidor remoto
void ProxyServer::SendRequestRemote(const char *req_string, int remote_socket, int buff_length){
  string temp;
  temp.append(req_string);

  int totalsent = 0;
  int senteach;
  cout << "SendRequestRemote: " << totalsent << " , " << buff_length << endl;

  while(totalsent < buff_length){
    cout << "about to send to remote" << endl;
    //Envia em parcelas (retorono do numero de bits enviado)
    if ((senteach = send(remote_socket, (void *) (req_string + totalsent), buff_length - totalsent, 0)) < 0) {
            cout << "error sending to the remote" << endl;
    } //if

    cout << "sent to remote" << senteach << endl;
    totalsent += senteach;
    cout << "total sent to remote: " << totalsent << endl;
  } //while
}

char* ProxyServer::RequestToString(RequestFields *req){
  //Set dos headers
  HeaderFields_set(req, "host", req->host);
  //Conexao nao persistente
  HeaderFields_set(req, "Connection", "close");

  int headersLen = HeaderFields_headersLen(req);
  char *headersBuf;
  headersBuf = (char*)malloc(headersLen + 1);
  RequestFields_unparse_headers(req, headersBuf, headersLen);
  headersBuf[headersLen] = '\0';

  int request_size = strlen(req->method) + strlen(req->path) + strlen(req->version) + headersLen + 4;
  char *serverReq;
  serverReq = (char *)malloc(request_size + 1);
  serverReq[0] = '\0';

  strcpy(serverReq, req->method);
  strcat(serverReq, " ");
  strcat(serverReq, req->path);
  strcat(serverReq, " ");
  strcat(serverReq, req->version);
  strcat(serverReq, "\r\n");
  strcat(serverReq, headersBuf);

  free(headersBuf);

  return serverReq;
}
