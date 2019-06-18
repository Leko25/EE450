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
#define HOST "localhost"

/**
 @return client transaction ID in the range [0 - 255]
*/
int getTransactionID(){
  int n = rand() % MAX;
  return n;
}

int main(void){
  srand(time(NULL));
  //Beej's procedure to setup TCP and create client socket
  int sockfd, rv, numBytes;
  struct addrinfo hints, *servinfo, *p;
  char assignedIP[BUFFERLEN];

  printf("using: localhost\n");

  memset(&hints, 0, sizeof(hints)); //ensure struct is empty
  hints.ai_family = AF_INET; //IPv4
  hints.ai_socktype = SOCK_STREAM; //TCP
  if((rv = getaddrinfo(HOST, PORT, &hints, &servinfo)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  //loop through all the results and connect to the first that we can
  for(p = servinfo; p != NULL; p = p->ai_next){
    if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      close(sockfd);
      perror("client: socket");
      continue;
    }

    if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
      close(sockfd);
      perror("client: connect");
      continue;
    }
    break;
  }
  if(p == NULL){
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  //Send transaction ID to DHCP server -- Discovery Phase
  int id = getTransactionID();
  send(sockfd, (char *)&id, sizeof(id), 0);

  //Receive DHCP transactionID and offered IP -- Offer Phase
  if(recv(sockfd, (char *)&id, sizeof(id), 0) == -1){
    perror("client: recv transaction ID");
    exit(1);
    return 1;
  }
  printf("offered phase transaction ID: %d\n", id);

  if((numBytes = recv(sockfd, assignedIP, BUFFERLEN, 0)) == -1){
    perror("client: recv assigned IP");
    exit(1);
    return 1;
  }
  assignedIP[numBytes] = '\0';

  //Change IP to offered IP and send request -- Request Phase
  if(inet_pton(AF_INET, assignedIP, &(p->ai_addr)) <= 0){
    perror("client: request");
    exit(1);
    return 1;
  }

  id = getTransactionID(); //new transaction ID
  //send transaction ID to DHCP server
  send(sockfd, (char *)&id, sizeof(id), 0);
  sleep(1);
  send(sockfd, assignedIP, INET_ADDRSTRLEN, 0); //echo IP
  printf("request phase assigned IP: %s\n", assignedIP);

  //Receive acknowledgement from DHCP server -- Acknowledgement Phase
  if(recv(sockfd, (char *)&id, sizeof(id), 0) == -1){
    perror("client: recv transaction ID");
    exit(1);
    return 1;
  }

  char echoIP[INET6_ADDRSTRLEN];
  if((numBytes = recv(sockfd, echoIP, BUFFERLEN, 0)) == -1){
    perror("client: acknowledgement phase echo IP");
    exit(1);
    return 1;
  }
  echoIP[numBytes] = '\0';
  printf("ack phase transactionID: %d\n", id);
  printf("acknowledgement phase echo IP: %s\n", echoIP);
  printf("\n");

  char testIP[INET6_ADDRSTRLEN];
  inet_ntop(p->ai_family, &p->ai_addr, testIP, sizeof(testIP));
  printf("Just to confirm new IP --> %s\n", testIP);
  freeaddrinfo(servinfo); //free up linked list
  close(sockfd);
  return 0;
}
