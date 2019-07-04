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

#define PORT "3432"
#define NO_DEPARTMENTS 3
#define LINE_SIZE 7
#define HOST "localhost"

//Get port number
int get_in_port(struct sockaddr *sa){
  if(sa->sa_family == AF_INET){
    return ntohs(((struct sockaddr_in *) sa)->sin_port);
  }else{
    return ntohs(((struct sockaddr_in6 *) sa)->sin6_port);
  }
}

//Define Node structure to store program names and their GPA.
struct Node{
  char * dept;
  struct Node * next;
};

//Append elements to list -- O(n)
void _append(struct Node ** list, char * p){
   struct Node * newNode = (struct Node *)malloc(sizeof(struct Node));
   newNode->dept = (char *)malloc(strlen(p) + 1); //talk to Xuan about this for grader -- previous malloc(strlen(p) + 1)
   strcpy(newNode->dept, p);
   newNode->next = NULL;
   if(*list == NULL){
     *list = newNode;
     return;
   }

   struct Node * tmpNode = *list;
   while(tmpNode->next != NULL){
     tmpNode = tmpNode->next;
   }
   tmpNode->next = newNode;
}

//TESTING utiliy function
void _printList(struct Node * list){
  struct Node * tmp = list;
  while(tmp != NULL){
    printf("%s", tmp->dept);
    tmp = tmp->next;
  }
}

//Reads file and creates a linked list with its contents
void _readFile(struct Node ** list, char * fileName){
  FILE *file;
  char ch;
  file = fopen(fileName, "r"); //read file
  if(file == NULL){
    perror("Error while opening the file.\n");
    exit(EXIT_FAILURE);
  }
  while((ch = fgetc(file)) != EOF){
    char data[LINE_SIZE];
    int ii_data = 0;
    while(ch != '\n'){
      data[ii_data++] = ch;
      ch = fgetc(file);
    }
    data[LINE_SIZE - 1] = '\0';
    _append(list, data);
  }
  fclose(file);
}

//Free up file contents once done with them
void _freeFile(struct Node ** list){
  while(*list != NULL){
    struct Node *trash = *list;
    *list = (*list)->next;
    free(trash);
  }
}


int main(void){
  int sockfd, rv;
    char dept_ip[INET6_ADDRSTRLEN]; //Department IP address
    unsigned int dept_port; //Department port
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in my_addr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if((rv = getaddrinfo(HOST, PORT, &hints, &servinfo)) != 0){
      fprintf(stderr, "\ngetaddrinfo: %s\n", gai_strerror(rv));
      return 1;
    }

    //loop through all the results and connect to the first that we can find
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
      return 1;
    }
    //1) Upon startup of Phase 1
    socklen_t len = sizeof(my_addr);
    getsockname(sockfd, (struct sockaddr *)&my_addr, &len);
    inet_ntop(AF_INET, &my_addr.sin_addr, dept_ip, sizeof(dept_ip));
    dept_port = ntohs(my_addr.sin_port);
    printf("<DepartmentC> has TCP port %d ", dept_port);
    printf("and IP address %s for Phase 1\n", dept_ip);

    //2) Upon establishing a TCP connection to the admission office
    printf("<DepartmentC> is now connected to the admission office\n");


    //readfile and send contents to Addmissions office
    struct Node * fileContent = NULL;
    _readFile(&fileContent, "DepartmentC.txt");

    struct Node * fileIter = fileContent;
    while(fileIter != NULL){//main Send loop
      sleep(5);
      send(sockfd, fileIter->dept, (int)strlen(fileIter->dept), 0);
      printf("<DepartmentC> has sent %s to the admission office\n", fileIter->dept);
      fileIter = fileIter->next;
    }
    char *ex = ":exit";
    sleep(5);
    send(sockfd, ex, (int)strlen(ex), 0);
    _freeFile(&fileContent);
    freeaddrinfo(servinfo); // free up list structure
    close(sockfd);
    printf("Updating the admission office is done for <DepartmentC>\n");
    printf("End of Phase 1 for <DepartmentC>\n");
}
