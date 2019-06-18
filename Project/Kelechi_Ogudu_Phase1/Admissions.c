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
#include <signal.h>
#include <sys/wait.h>
#include "uthash.h" //Used for building hash map

#define PORT "3432"
#define HOST "localhost"
#define MAXDATASIZE 20
#define DEPARTMENT_SIZE 3
#define BACKLOG 3

//Department names
char * department_names[] = {"DepartmentA", "DepartmentB", "DepartmentC"};

//wait for the child whose process ID is equal to the value of pid
//Used in reaping zombie processes
void sigchild_handler(int s){
  pid_t p;
  int status;
  while((p = waitpid(-1, &status, WNOHANG)) != -1);
}

//Get port number
int get_in_port(struct sockaddr *sa){
  if(sa->sa_family == AF_INET){
    return ntohs(((struct sockaddr_in *) sa)->sin_port);
  }else{
    return ntohs(((struct sockaddr_in6 *) sa)->sin6_port);
  }
}

//Create hashable structure to store departments and gpa requirements
struct my_struct{
  const char * progName;  /** key */
  double gpa;       /** value */
  UT_hash_handle hh;   //This makes the structure hashable
};

struct my_struct *department = NULL;

//---------------------- Hash Functions ------------------------------------//
void _add_dept(char * dept, double grade){
  struct my_struct *s = NULL;
  HASH_FIND_STR(department, dept, s); // find if key already exists in the hash
  if(s == NULL){
    //Initialize struct
    s = (struct my_struct *)malloc(sizeof(struct my_struct));
    s->progName = dept;
    s->gpa = grade;
    //this adds the address of the key being added -- dept: gpa
    HASH_ADD_KEYPTR(hh, department, s->progName, strlen(s->progName), s);
  }
}

double _get_gpa(char * dept){
  struct my_struct *s;

  HASH_FIND_STR(department, dept, s); // s: output pointer
  return s->gpa;
}

//For testing
void _print_dept(){
  struct my_struct *s;
  for(s = department; s != NULL; s = (struct my_struct *)(s->hh.next)){
    printf("%s: %.1f\n", s->progName, s->gpa);
  }
}

//Get number of keys
int _get_numberKeys(){
  if(department == NULL){
    return 0;
  }else{
    return HASH_COUNT(department);
  }
}

//Delete all elements within the hash
void _delete_all(){
  struct my_struct *curr, *tmp;
  HASH_ITER(hh, department, curr, tmp){
    HASH_DEL(department, curr);
    free(curr);
  }
}

//Hardcoded --- Parse message from department
void _parse_dept(char * msg, char * s, double * gp){
  char gpa[4];
  double d_gpa;
  int ii;
  for(ii = 0; ii < strlen(msg); ii++){
    if(msg[ii] == '#'){
      break;
    }
    s[ii] = msg[ii];
  }
  s[ii] = '\0';

  int jj;
  for(jj = 0; jj < 4; jj++){
    gpa[jj] = msg[++ii];
  }
  gpa[jj - 1] = '\0';
  d_gpa = atof(gpa);
  *gp = d_gpa;
}

int main(void){
  int sockfd, rv, child, numBytes;
  int opt = 1;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; //connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int status = 0;
  int counter = BACKLOG;

  //Admissions information
  struct sockaddr_in my_addr;
  char admin_ip[INET6_ADDRSTRLEN]; //Admissions IP address
  unsigned int admin_port; //Admissions port

  //Department information
  char dept[MAXDATASIZE];
  double gpa;
  int count = 0;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if((rv = getaddrinfo(HOST, PORT, &hints, &servinfo)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  //loop though all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next){
    if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      perror("server: socket");
      continue; //move to next available socket
    }

    //reuse port and supress address already in use warnings
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1){
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
  socklen_t len = sizeof(my_addr);
  getsockname(sockfd, (struct sockaddr *)&my_addr, &len);
  inet_ntop(AF_INET, &my_addr.sin_addr, admin_ip, sizeof(admin_ip));
  admin_port = ntohs(my_addr.sin_port);
  printf("The admission office has TCP port %d and IP address %s\n", admin_port, admin_ip);

  freeaddrinfo(servinfo); //free list structure

  //Listen to client
  if(listen(sockfd, BACKLOG) == -1){
    perror("server: listen");
    exit(1);
  }

  //Reap all dead processes
  sa.sa_handler = sigchild_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if(sigaction(SIGCHLD, &sa, NULL) == -1){
    perror("sigaction");
    exit(1);
  }

  while(1){//accept() main loop
    sin_size = sizeof(their_addr);
    child = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if(child == -1){
      perror("server: accept");
      continue;
    }
    counter--;
    count++;
    if(!fork()){//this is the child process
      close(sockfd);
      while(1){
        if((numBytes = recv(child, dept, MAXDATASIZE, 0)) == -1){
          perror("server: recv");
          exit(1);
        }
        dept[numBytes] = '\0';
        if(strcmp(dept, ":exit") == 0){
          if(_get_numberKeys() == DEPARTMENT_SIZE){
            printf("Received the program list from <%s>\n", department_names[count - 1]);
          }
          break;
        }
        else{
          char *dept_name = NULL;
          dept_name = (char *)calloc(DEPARTMENT_SIZE, sizeof(char));
          _parse_dept(dept, dept_name, &gpa);
          _add_dept(dept_name, gpa);
          bzero(dept, sizeof(dept));
        }
      }//end while
      _print_dept();
      close(child);
      exit(0);
    }// end fork
    if(counter > 0){
      close(child); //parent doesn't need this
      continue;
    }else{
      wait(&status);
      printf("End of Phase 1 for the admission office\n");
      break;
    }
  }
  return 0;
}
