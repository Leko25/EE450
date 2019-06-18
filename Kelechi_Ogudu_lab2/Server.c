//Name: Kelechi Ogudu
//USC_ID: 5413285132

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT "3432"
#define MAX 255
#define BUFFERLEN 20
#define BACKLOG 3
#define HOST "localhost"

//get sockaddr, IPv4, IPv6:
void * get_in_addr(struct sockaddr *sa){//Beej's code
  if(sa->sa_family == AF_INET){
    return &(((struct sockaddr_in *) sa)->sin_addr);
  }else{
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
  }
}

/**
 @returns generated id length
*/
int getIDLength(int id){
  int count = 0;
  if(id == 0){
    return 1;
  }

  while(id >= 1){
    count++;
    id /= 10;
  }
  return count;
}


/**
 concat two strings
*/
char * concat(const char * s1, const char * s2){
  char * result = malloc(strlen(s1) + strlen(s2) + 1);
  strcpy(result, s1);
  strcat(result, s2);
  return result;
}


/**
 @return client transaction ID in the range [0 - 255]
*/
char * getTransactionID(){
  //Create empty string
  char *result = NULL;
  int lenResult = 1;
  result = (char *)calloc(lenResult, sizeof(char));
  srand(time(NULL));
  for(int ii = 0; ii <= 2; ii++){
    int n = rand() % MAX;
    int idLen = getIDLength(n);
    char * id = malloc(idLen + 2);
    sprintf(id, "%d.", n);
    result = concat(result, id);
  }
  return result;
}

int getRandomID(){
  return rand() % MAX;
}


int main(void){
  //Beej's procedure to setup TCP and create server socket
  int sockfd, child, rv, numBytes;
  int opt = 1;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; //connector's address information
  socklen_t sin_size;
  char s[INET6_ADDRSTRLEN];

  //Intialize empty IP address to be offered once client sends transaction ID
  char * completeIP = NULL;
  int numOfChar = 1;
  completeIP = (char *)calloc(numOfChar, sizeof(char));

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; //enforce IPv4
  hints.ai_socktype = SOCK_STREAM; //TCP
  hints.ai_flags = AI_PASSIVE; //use my IP

  if((rv = getaddrinfo(HOST, PORT, &hints, &servinfo)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  //loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next){
    if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      perror("server: socket");
      continue; //move to next available socket
    }

    //reuse port and supress address already in use warnings
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int)) == -1){
      perror("server: setsockopt");
      exit(1);
    }

    //Bind socket and local address
    if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
      close(sockfd);
      perror("server: bind");
      continue;
    }
    break;
  }

  if(p == NULL){
    fprintf(stderr, "server: failed to bind\n");
    return 1;
  }

  freeaddrinfo(servinfo); //free list structure

  //Listen to client
  if(listen(sockfd, BACKLOG) == -1){
    perror("server: listen");
    exit(1);
  }

  //Accept client and create child socket
  sin_size = sizeof(their_addr);
  if((child = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1){
    perror("server: accept");
    exit(1);
  }

  inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
  printf("server: got connection from %s\n", s);

  //Discovery Phase
  int id = 0;

  if(recv(child, (char *)&id, sizeof(id), 0) == -1){
    perror("server: recv transaction ID");
    exit(1);
    return 1;
  }
  printf("discovery phase transaction ID: %d\n", id);

  //concatenate transaction id from client and random generated ip
  char * incompleteIP = getTransactionID();
  char * idChar = malloc(getIDLength(id) + 1);
  sprintf(idChar, "%d", id);
  completeIP = concat(incompleteIP, idChar);

  //Offer Phase
  id = getRandomID();
  send(child, (char *)&id, sizeof(id), 0);
  sleep(1);
  send(child, completeIP, (int)strlen(completeIP), 0);

  //Request Phase
  if(recv(child, (char *)&id, sizeof(id), 0) == -1){
    perror("server: request transaction ID");
    exit(1);
    return 1;
  }
  char echoIP[INET6_ADDRSTRLEN];

  if((numBytes = recv(child, echoIP, INET_ADDRSTRLEN, 0)) == -1){
    perror("server: request echo IP");
    exit(1);
    return 1;
  }
  echoIP[numBytes] = '\0';
  printf("request phase transaction ID: %d\n", id);
  printf("request phase echo IP: %s\n", echoIP);

  //Acknowledgemnt Phase
  id = getRandomID();
  send(child, (char *)&id, sizeof(id), 0);
  sleep(1);
  send(child, echoIP, (int)strlen(echoIP), 0);
  printf("%s\n", completeIP);

  close(child); //close connection with child
  return 0;
}
