#include "../include/ProxyParser.h"

#define MAX_REQ_LEN 65535
#define MIN_REQ_LEN 4

#define DEFAULT_NHDRS 8
#define MAX_CONNECTIONS 20

static const char *root_abs_path = "/";

/* Funções internas ao módulo */

//Se debug for setado em 1, imprime os erros
void debug(const char * format, ...) {
     va_list args;
     if (DEBUG) {
	  va_start(args, format);
	  vfprintf(stderr, format, args);
	  va_end(args);
     }
}

// Print de uma linha de requisição
int RequestFields_printRequestLine(RequestFields *req_fields, char *buf, size_t buflen,
size_t *tmp);

// Obtém tamanho da linha de requisição
size_t RequestFields_requestLineLen(RequestFields *req_fields);

/* Funções públicas */

int HeaderFields_set(RequestFields *req_fields, const char *key, const char *value){

  HeaderFields *hf;
  HeaderFields_pop(req_fields, key);

  // Se o que esta sendo usado eh maior que o previamente estabelecido, dobra-se a capacidade da lista
  if(req_fields->headerslen <= req_fields->headersused+1){
    req_fields->headerslen = req_fields->headerslen*2;
    req_fields->headers = (HeaderFields *) realloc(req_fields->headers,
    req_fields->headerslen * sizeof(HeaderFields));

    if(!req_fields->headers)
      return -1;

  }

  hf = req_fields->headers + req_fields->headersused;
  req_fields->headersused = req_fields->headersused + 1;

  //Aloca espaco e copia a chave
  hf->key = (char *)malloc(strlen(key)+1);
  memcpy(hf->key, key, strlen(key));
  //Termina com \0
  hf->key[strlen(key)] = '\0';

  if(value){
    hf->value = (char *)malloc(strlen(value)+1);
    memcpy(hf->value, value, strlen(value));
    hf->value[strlen(value)] = '\0';

    hf->keylen = strlen(key)+1;
    hf->valuelen = strlen(value)+1;
  }

  return 0;
}

HeaderFields* HeaderFields_get(RequestFields *req_fields, const char *key){
  size_t i = 0;
  HeaderFields *tmp;

  while(req_fields->headersused > i){
    tmp = req_fields->headers + i;

    if(tmp->key && key && strcmp(tmp->key, key) == 0){
      return tmp;
    }

    i++;
  }

  // Se nao encontrar nada
  return NULL;
}

int HeaderFields_pop(RequestFields *req_fields, const char *key){
  HeaderFields *tmp;
  tmp = HeaderFields_get(req_fields, key);

  if(tmp == NULL){
    return -1;
  }

  free(tmp->key);
  free(tmp->value);
  tmp->key = NULL;

  return 0;
}

/* Métodos privados para as linhas de cabeçalho */

void HeaderFields_create(RequestFields *req_fields){
  req_fields->headers = (HeaderFields *)malloc(sizeof(HeaderFields)*DEFAULT_NHDRS);
  req_fields->headerslen = DEFAULT_NHDRS;
  req_fields->headersused = 0;
}

size_t HeaderFields_lineLen(HeaderFields *hf){
  if(hf->key != NULL){
    // +4 do \r\n
    return strlen(hf->key) + strlen(hf->value)+4;
  }
  return 0;
}

size_t HeaderFields_headersLen(RequestFields *req_fields){
  if(!req_fields || !req_fields->buf){
    return 0;
  }

  size_t i = 0;
  int len = 0;

  while(req_fields->headersused > i){
    len += HeaderFields_lineLen(req_fields->headers + i);
    i++;
  }

  len += 2;
  return len;
}

int HeaderFields_printHeaders(RequestFields *req_fields, char *buf, size_t len){
  char *curr = buf;
  HeaderFields *current_header;
  size_t i = 0;

  if(len < HeaderFields_headersLen(req_fields)){
    debug("Buffer to print headers too short\n");
    return -1;
  }

  while(req_fields->headersused > i){
    current_header = req_fields->headers + i;
    if(current_header->key){
      memcpy(curr, current_header->key, strlen(current_header->key));
      memcpy(curr+strlen(current_header->key), ": ", 2);
      memcpy(curr+strlen(current_header->key)+2, current_header->value,
    strlen(current_header->value));
      memcpy(curr+strlen(current_header->key)+2+strlen(current_header->value), "\r\n", 2);
      //Aponta pro fim da string copiada
      curr += strlen(current_header->key)+strlen(current_header->value)+4;
    } //if
    i++;
  } //while
  memcpy(curr, "\r\n", 2);
  return 0;
}

void HeaderFields_destroySingleLine(HeaderFields *hf){
  if(hf->key != NULL){
    free(hf->key);
    hf->key = NULL;
    free(hf->value);
    hf->value = NULL;
    hf->keylen = 0;
    hf->valuelen = 0;
  }
}

void HeaderFields_destroy(RequestFields *req_fields){
  size_t i = 0;

  //Destroi lista 1 por 1
  while(req_fields->headersused > i){
    HeaderFields_destroySingleLine(req_fields->headers+i);
    i++;
  }

  req_fields->headersused = 0;
  free(req_fields->headers);
  req_fields->headerslen = 0;
}

// Faz o parse de uma linha por vez
int HeaderFields_parse(RequestFields *req_fields, char *line){
  char *key;
  char *value;
  char *aux1;
  char *aux2;

  //Retorna posicao do primeiro char de interesse nessa string
  aux1 = strchr(line, ':');
  //Se ptr retornado eh null, n tem dois pontos;
  if(aux1 == NULL){
    debug("Couldn't find colon\n");
    return -1;
  }

  //Chave tera o tamanho do inicio ate os dois pontos
  key = (char *)malloc((aux1-line+1)*sizeof(char));
  memcpy(key, line, aux1-line);
  key[aux1-line] = '\0';

  //Pula espaco e vai pro simbolo
  aux1 += 2;
  aux2 = strstr(aux1, "\r\n");
  value = (char*)malloc((aux2-aux1+1)*sizeof(char));
  memcpy(value, aux1, (aux2-aux1));
  value[aux2-aux1] = '\0';

  HeaderFields_set(req_fields, key, value);
  free(key);
  free(value);
  return 0;
}

/******* Métodos Públicos *******/

void RequestFields_destroy(RequestFields *pr)
{
    if(pr->buf != NULL){
	     free(pr->buf);
    }
    if (pr->path != NULL) {
	     free(pr->path);
    }
    if(pr->headerslen > 0){
	     HeaderFields_destroy(pr);
    }

    free(pr);
}

// Criacao de uma estrutura de requisicao vazia
RequestFields* RequestFields_create(){
  RequestFields *req_fields;

  req_fields = (RequestFields*)malloc(sizeof(RequestFields));

  if(req_fields != NULL){
    HeaderFields_create(req_fields);
    req_fields->buf = NULL;
    req_fields->buflen = 0;
    req_fields->host = NULL;
    req_fields->method = NULL;
    req_fields->protocol = NULL;
    req_fields->path = NULL;
    req_fields->version = NULL;
    req_fields->port = NULL;
  }

  return req_fields;
}

// Criacao da string de volta a partir da estrutura parseada
int RequestFields_unparse(RequestFields *req_fields, char *buf, size_t buflen){
  if(!req_fields || !req_fields->buf){
    return -1;
  }

  size_t tmp;
  if(RequestFields_printRequestLine(req_fields, buf, buflen, &tmp) < 0){
    return -1;
  }
  if(HeaderFields_printHeaders(req_fields, buf, buflen-tmp)< 0){
    return -1;
  }
  return 0;
}

//Unparse dos cabecalhos em um buffer ja alocado
int RequestFields_unparse_headers(RequestFields *req_fields, char *buf, size_t buflen){
  if(!req_fields || !req_fields->buf){
    return -1;
  }

  if(HeaderFields_printHeaders(req_fields, buf, buflen)< 0){
    return -1;
  }

  return 0;
}

// Cálculo do tamanho de uma requisição
size_t RequestFields_totalLen(RequestFields *req_fields){
  if(!req_fields || !req_fields->buf){
    return 0;
  }
  return RequestFields_requestLineLen(req_fields)+HeaderFields_headersLen(req_fields);
}

/*  Parse do buffer de requisição
  Parametros:
  - parse: ponteiro para o objeto criado
  - buf: ptr pro buffer com o request a ser criado
  - buflen: tamanho do buffer, incluindo o ultimo \r\n\r\n

  - Retorna -1 se falhar, 0 se for bem-sucedido
*/
int RequestFields_parse(RequestFields *parse, const char *buf, int buflen){
  char *full_addr;
  char *saveptr;
  char *index;
  char *currentHeader;

  if(parse->buf != NULL){
    debug("Parse Object already has a request\n");
    return -1;
  }

  if(buflen < MIN_REQ_LEN || buflen > MAX_REQ_LEN){
    debug("Invalid size for buflen %d\n", buflen);
    return -1;
  }

  char *tmp_buf = (char *)malloc(buflen + 1);
  memcpy(tmp_buf, buf, buflen);
  tmp_buf[buflen] = '\0';

  index = strstr(tmp_buf, "\r\n\r\n");
  if(index == NULL){
    debug("Invalid request, no end of header\n");
    free(tmp_buf);
    return -1;
  }

  //Copia a linha de requisicao para parse->buf
  index = strstr(tmp_buf, "\r\n");
  if(parse->buf == NULL){
    parse->buf = (char *) malloc((index-tmp_buf)+1);
    parse->buflen = (index-tmp_buf)+1;
  }
  memcpy(parse->buf, tmp_buf, index - tmp_buf);
  parse->buf[index - tmp_buf] = '\0';

  //Parse da linha de requisicao
  parse->method = strtok_r(parse->buf, " ", &saveptr);
  if(parse->method == NULL){
    debug("Invalid request line, no space\n");
    free(tmp_buf);
    free(parse->buf);
    parse->buf = NULL;
    return -1;
  }

  if (strcmp (parse->method, "GET")) {
	  debug( "invalid request line, method not 'GET': %s\n",
		parse->method);
	  free(tmp_buf);
	  free(parse->buf);
	  parse->buf = NULL;
	  return -1;
}

  full_addr = strtok_r(NULL, " ", &saveptr);

  if(full_addr == NULL){
    debug("Invalid request line, no full url\n");
    free(tmp_buf);
    free(parse->buf);
    parse->buf = NULL;
    return -1;
  }

  parse->version = full_addr + strlen(full_addr) + 1;

  if (parse->version == NULL){
    debug("Invalid request line, version missing!\n");
    free(tmp_buf);
    free(parse->buf);
    parse->buf = NULL;
    return -1;
  }

  if (strncmp (parse->version, "HTTP/", 5)) {
	  debug("Invalid request line, unsupported version %s\n",parse->version);
	  free(tmp_buf);
	  free(parse->buf);
	  parse->buf = NULL;
	  return -1;
}

  parse->protocol = strtok_r(full_addr, "://", &saveptr);
  if (parse->protocol == NULL){
    debug("Invalid request line, host missing!\n");
    free(tmp_buf);
    free(parse->buf);
    parse->buf = NULL;
    return -1;
  }

  const char *rem = full_addr + strlen(parse->protocol) + strlen("://");
  // Tamanho do endereco absoluto
  size_t abs_uri_len = strlen(rem);

  parse->host = strtok_r(NULL, "/", &saveptr);
  if(parse->host == NULL){
    debug("Invalid request line, missing host\n");
    free(tmp_buf);
    free(parse->buf);
    parse->buf = NULL;
    return -1;
  }

  if(strlen(parse->host) == abs_uri_len){
    debug("Invalid request line, missing absolute path\n");
    free(tmp_buf);
    free(parse->buf);
    parse->buf = NULL;
    return -1;
  }

  parse->path = strtok_r(NULL, " ", &saveptr);
  if(parse->path == NULL){
    int rlen = strlen(root_abs_path);
    parse->path = (char *)malloc(rlen+1);
    strncpy(parse->path, root_abs_path, rlen +1);
  } else if(strncmp(parse->path, root_abs_path, strlen(root_abs_path)) == 0){
    debug("Invalid request line, path cannot begin with two slash characters\n");
    free(tmp_buf);
    free(parse->buf);
    parse->buf = NULL;
    //Zerar tbm o path, pq colocou um path errado
    parse->path = NULL;
    return -1;
  } else{
      //copia parse->path, com um / de prefixo
      char *tmp_path = parse->path;
      int rlen = strlen(root_abs_path);
      int plen = strlen(parse->path);
      parse->path = (char *)malloc(rlen + plen +1);
      strncpy(parse->path, root_abs_path, rlen);
      strncpy(parse->path + rlen, tmp_path, plen+1);
  }

  parse->host = strtok_r(parse->host, ":", &saveptr);
  parse->port = strtok_r(NULL, "/", &saveptr);

  if (parse->host == NULL){
    debug("Invalid request line, missing host\n");
    free(tmp_buf);
    free(parse->buf);
    free(parse->path);
    parse->buf = NULL;
    parse->path = NULL;
    return -1;
  }

  if (parse->port != NULL) {
    int port = strtol(parse->port, (char **)NULL, 10);
    // errno eh uma variable global que retorna erro em caso de problema
    if(port == 0 && errno == EINVAL){
      debug("Invalid request line, bad port: %s\n", parse->port);
      free(tmp_buf);
  	  free(parse->buf);
  	  free(parse->path);
  	  parse->buf = NULL;
  	  parse->path = NULL;
      return -1;
    } //if port
  } // if parse->port


  // Parse dos cabeçalhos
  int erro = 0;
  // Pula a request line
  currentHeader = strstr(tmp_buf, "\r\n")+2;

  //Enquanto n chegar ao fim dos cabecalhos
  while(currentHeader[0] != '\0' && !(currentHeader[0] == '\r' && currentHeader[1] =='\n')) {
    // Se obtiver sucesso, retorna 0
    if(HeaderFields_parse(parse, currentHeader)){
      erro = -1;
      return -1;
    }

    currentHeader = (strstr(currentHeader, "\r\n"));
    if(currentHeader == NULL || strlen(currentHeader) < 2){
      break;
    }

    currentHeader += 2;
  } //while

  free(tmp_buf);
  return erro;
} // funcao parser]


/* Metodos Privado do RequestFields */

size_t RequestFields_requestLineLen(RequestFields *req_fields){
  if(!req_fields || !req_fields->buf){
    return 0;
  }

  size_t len = strlen(req_fields->method) + 1 + strlen(req_fields->protocol) + 3 + strlen(req_fields->host) + 1 + strlen(req_fields->version) + 2;

  if(req_fields->port != NULL){
    len += strlen(req_fields->port)+1;
  }

  len += strlen(req_fields->path);
  return len;
}

int RequestFields_printRequestLine(RequestFields *pr,char * buf, size_t buflen, size_t *tmp)
{
    char * current = buf;

    if(buflen <  RequestFields_requestLineLen(pr)){
	     debug("not enough memory for first line\n");
	     return -1;
    }
    memcpy(current, pr->method, strlen(pr->method));
    current += strlen(pr->method);
    current[0]  = ' ';
    current += 1;

    memcpy(current, pr->protocol, strlen(pr->protocol));
    current += strlen(pr->protocol);
    memcpy(current, "://", 3);
    current += 3;
    memcpy(current, pr->host, strlen(pr->host));
    current += strlen(pr->host);
    if(pr->port != NULL){
	    current[0] = ':';
	    current += 1;
	    memcpy(current, pr->port, strlen(pr->port));
	    current += strlen(pr->port);
    }

    memcpy(current, pr->path, strlen(pr->path));
    current += strlen(pr->path);

    current[0] = ' ';
    current += 1;

    memcpy(current, pr->version, strlen(pr->version));
    current += strlen(pr->version);
    memcpy(current, "\r\n", 2);
    current +=2;
    *tmp = current-buf;
    return 0;
}
