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
#define NO_DEPARTMENTS 3
#define HOST "localhost"
#define UDP_PORT "21332"

//Global variable containing respective department filename and extension
// std::string filenames[] = {"DepartmentB.txt", "DepartmentB.txt", "DepartmentC.txt"};
// std::string department_names[] = {"DepartmentB", "DepartmentB", "DepartmentC"};
// std::string department_ports[] = {"21232", "21332", "21432"};

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
      perror("client: socket");
      continue;
    }
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
    return 1;
  }
  freeaddrinfo(servinfo);
  // char file[15];
  // strcpy(file, filenames[ii].c_str());
  // const char * dept = department_names[ii].c_str();
  len = sizeof(my_addr);
  getsockname(sockfd, (struct sockaddr *)&my_addr, &len);
  inet_ntop(AF_INET, &my_addr.sin_addr, s, sizeof(s));
  printf("<DepartmentB> has TCP port %d and IP address %s for Phase 1\n", (int)ntohs(my_addr.sin_port), s);
  printf("<DepartmentB> is now connected to the admissions office\n");

  //Send packets
  FILE *dept_file;
  char line[15];
  dept_file = fopen("DepartmentB.txt", "r");
  if(dept_file != NULL){
    for(int jj = 0; jj < 3; jj++){
      fgets(line, 15, dept_file);
      char * dept_info = strtok(line, "\n");
      if(send(sockfd, dept_info, (int)strlen(dept_info), 0) == 1){
        perror("send\n");
      }
      printf("<DepartmentB> has sent %s to the admission office\n", dept_info);
    }
  }
  else{
    printf("Error while opening the file.\n");
  }
  fclose(dept_file);
  close(sockfd);
  printf("End of Phase 1 for <DepartmentB>\n");

  // char port[10];
  // char dept[15];
  // strcpy(port, department_ports[ii].c_str());
  // strcpy(dept, department_names[ii].c_str());
  char buff[100];
  int numBytes;
  struct sockaddr_storage their_addr;
  socklen_t addr_len;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;
  if ((rv = getaddrinfo(HOST, UDP_PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("listener: socket");
      continue;
    }
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("listener: bind");
      continue;
    }
    break;
  }
  if (p == NULL) {
    fprintf(stderr, "listener: failed to bind socket\n");
    return 2;
  }
  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
  printf("<DepartmentB> has UDP port %s and IP address %s for Phase2\n", UDP_PORT, s);
  freeaddrinfo(servinfo);

  addr_len = sizeof(their_addr);
  numBytes = recvfrom(sockfd, buff, 99, 0, (struct sockaddr *)&their_addr, &addr_len);
  while(numBytes != 8){
    if(numBytes == -1){
      perror("department: recvfrom");
      exit(1);
    }
    buff[numBytes] = '\0';
    printf("<Student%c> has been admitted to DepartmentB\n", buff[7]);
    numBytes = recvfrom(sockfd, buff, 99, 0, (struct sockaddr *)&their_addr, &addr_len);
  }
  printf("End of Phase2 for DepartmentB\n");
  close(sockfd);
  return 0;
}
