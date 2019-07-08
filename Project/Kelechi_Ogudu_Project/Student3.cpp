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
#include <signal.h>
#include <sys/wait.h>
#include <string>

#define PORT "3432"
#define HOST "localhost"
#define NO_STUDENTS 5
#define STUDENT_MAXDATA 20
#define UDP_PORT "21732"
//Beejs - get sockaddr, IPv4 IPv6
void *get_in_addr(struct sockaddr * sa){
  if(sa->sa_family == AF_INET){
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }else{
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
  }
}

int main(void){
  //Beej's setup TCP connections
  struct addrinfo hints, *servinfo, *p;
  int sockfd, rv;
  struct sockaddr_in my_addr;
  socklen_t len;
  char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if((rv = getaddrinfo(HOST, PORT, &hints, &servinfo)) != 0){
    fprintf(stderr, "\ngetaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  for(p = servinfo; p != NULL; p = p->ai_next){
    if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      close(sockfd);
      perror("student: socket");
      continue;
    }
    if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      close(sockfd);
      perror("student: socket");
      continue;
    }
    if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
      close(sockfd);
      perror("student: connect");
      continue;
    }
    break;
  }
  if(p == NULL){
    fprintf(stderr, "student: failed to connect\n");
    return 1;
  }
  freeaddrinfo(servinfo);
  len = sizeof(my_addr);
  getsockname(sockfd, (struct sockaddr *)&my_addr, &len);
  inet_ntop(AF_INET, &my_addr.sin_addr, s, sizeof(s));
  printf("<Student3> has TCP port %d and IP address %s\n", (int)ntohs(my_addr.sin_port), s);

  FILE *stuFile;
  char line[STUDENT_MAXDATA];
  stuFile = fopen("student3.txt", "r");
  if(stuFile != NULL){
    while(fgets(line, STUDENT_MAXDATA, stuFile)){
      char * std_info = strtok(line, "\n");
      if(send(sockfd, std_info, strlen(std_info), 0) == -1){
        perror("send: data\n");
      }
    }
    char flag[] = "finished";
    if(send(sockfd, flag, (int)strlen(flag), 0) == -1){
      perror("send: flag\n");
    }
    printf("Completed sending application for <Student3>\n");
    int numBytes;
    char buff[STUDENT_MAXDATA];
    if((numBytes = recv(sockfd, buff, 2, 0)) == -1){
      exit(1);
    }
    printf("<Student3> has received reply from the admissions office\n");
  }else{
    printf("Error while openinig the file.\n");
  }
  fclose(stuFile);
  close(sockfd);

  struct sockaddr_storage their_addr;
  socklen_t addr_len;
  int numBytes;
  char buff[100];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;
  if((rv = getaddrinfo(HOST, UDP_PORT, &hints, &servinfo)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("student udp: socket");
      continue;
    }
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("student udp: bind");
      continue;
    }
    break;
  }
  if (p == NULL) {
    fprintf(stderr, "student: failed to bind socket\n");
    return 1;
  }
  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
  printf("<Student3> has UDP port %s and IP address %s for Phase2\n", UDP_PORT, s);
  freeaddrinfo(servinfo);
  len = sizeof(their_addr);
  if((numBytes = recvfrom(sockfd, buff, 99, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1){
    perror("student: recvfrom");
    exit(1);
  }
  buff[numBytes] = '\0';
  printf("<Student3> has received the application result\n");
  printf("End of Phase 2 for <Student3>\n");
  close(sockfd);
  return 0;
}
