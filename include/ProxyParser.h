#ifndef PROXY_PARSER
#define PROXY_PARSER

#define DEBUG 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <ctype.h>

/* Esse módulo tem como objetivo fazer o parser do buffer de requisição
do browser, que contem a mensagem de requisição em HTTP
  Uma requisição HTTP contém uma request line com método, url e versão do protocolo;
  Linhas de cabeçalho separada por dois pontos e valor (serão colocados numa lista
ligada)
*/

/* Linhas de cabeçalho após a Request Line são da forma:
  "Chave:valor\r\n"
  Serao armazenados numa lista de HeaderFields no ParsedRequest
*/
typedef struct st_HeaderFields {
  char *key;
  size_t keylen;
  char *value;
  size_t valuelen;
} HeaderFields;

typedef struct st_ParsedRequest {
  char *method;
  char *protocol;
  char *host;
  char *port;
  char *path;
  char *version;
  char *buf;  // Request Line parseada
  size_t buflen;
  HeaderFields *headers;
  size_t headersused;
  size_t headerslen;
} RequestFields;

// Requisições

// Criacao de um objeto de parse vazio
RequestFields* RequestFields_create();

// Parse do buffer recebido em buf, com tamanho buflen
int RequestFields_parse(RequestFields *parse, const char *buf, int buflen);

// Destruição do objeto
void RequestFields_destroy(RequestFields *rf);

// Escreve de novo o buffer inteiro a partir de um objeto RequestFields
int RequestFields_unparse(RequestFields *rf, char *buf, size_t buflen);

// Recuperação do cabeçalho do buffer, sem a request Line
int RequestFields_unparse_headers(RequestFields *rf, char *buf, size_t buflen);

// Cálculo do comprimento total da requisição
size_t RequestFields_totalLen(RequestFields *rf);

// Somente cabeçalho

// Cálculo do comprimento dos headers + \r\n
size_t HeaderFields_headersLen(RequestFields *rf);

// Construcao do objeto Header pelas chaves + valores
int HeaderFields_set(RequestFields *rf, const char *key, const char *value);

// Obtencao de uma struct de cabecalhos a partir do objeto de requisicao
HeaderFields* HeaderFields_get(RequestFields *rf, const char *key);

// Remocao das chaves/valores do objeto
int HeaderFields_pop(RequestFields *rf, const char *key);


#endif
