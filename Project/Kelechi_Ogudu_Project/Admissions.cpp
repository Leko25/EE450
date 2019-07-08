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
#define STUDENT_MAXDATA 20
#define NO_STUDENTS 5
#define BACKLOG 20
#define HOST "localhost"
#define MAXDATA 15
#define LINE_SIZE 6

std::string department_names[] = {"DepartmentA", "DepartmentB", "DepartmentC"};
std::string department_ports[] = {"21232", "21332", "21432"};
std::string student_ports[] {"21532", "21632", "21732", "21832", "21932"};
std::string student_out[] = {"student1_prog.txt", "student2_prog.txt", "student3_prog.txt", "student4_prog.txt", "student5_prog.txt"};
std::string department_out[] = {"deptA_prog.txt", "deptB_prog.txt", "deptC_prog.txt"};
struct Node{
  char dept[10];
  double gpa;
  struct Node * next;
};

// struct Student{
//   double grade;
//   std::string interest[3];
// };

//Beej's reap dead processes
void sigchld_handler(int s){
  while(waitpid(-1, NULL, WNOHANG) > 0);
}

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
  int sockfd, child, rv;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr;
  socklen_t sin_size;

  struct sigaction sa;
  int yes = 1;
  char s[INET_ADDRSTRLEN];


  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if((rv = getaddrinfo(HOST, PORT, &hints, &servinfo)) != 0){
    fprintf(stderr, "\ngetaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for(p = servinfo; p != NULL; p = p->ai_next){
    if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      perror("server: socket\n");
      continue;
    }
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
      perror("setsockopt\n");
      exit(1);
    }
    if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
      close(sockfd);
      perror("server: bind\n");
      continue;
    }
    break;
  }
  if(p == NULL){
    fprintf(stderr, "server: failed to bind\n");
    return 1;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
  printf("The admissions office has TCP port%s and IP address %s\n", PORT, s);

  if(listen(sockfd, BACKLOG) == -1){
    perror("listen\n");
    exit(1);
  }

  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if(sigaction(SIGCHLD, &sa, NULL) == -1){
    perror("sigation\n");
    exit(1);
  }

  for(int ii = 0; ii < NO_DEPARTMENTS; ii++){
    sin_size = sizeof(their_addr);
    if((child = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1){
      perror("accept\n");
      continue;
    }
    if(!fork()){
      close(sockfd);

      int numBytes;
      const char * dept = department_names[ii].c_str();
      char buff[MAXDATA];
      FILE *file;
      file = fopen("Department.txt", "a");
      if(file != NULL){
        for(int jj = 0; jj < NO_DEPARTMENTS; jj++){
          if((numBytes = recv(child, buff, LINE_SIZE, 0)) == -1){
            exit(1);
          }
          buff[numBytes] = '\0';
          fprintf(file, "%s\n", buff);
        }
      }
      else{
        printf("failed to cache data\n");
      }
      printf("Received the program list from <%s>\n", dept);
      fclose(file);
      close(child);
      exit(0);
    }
    close(child);
  }
  int status1, status2, status3;
  wait(&status1);
  wait(&status2);
  wait(&status3);


  char line[MAXDATA];
  char * data;
  FILE *file;
  file = fopen("Department.txt", "r");
  struct Node * department = NULL;
  if(file != NULL){
    while(fgets(line, MAXDATA, file)){
      struct Node *tmp = (struct Node *)malloc(sizeof(struct Node));
      data = strtok(line, "#");
      strcpy(tmp->dept, data);
      data = strtok(NULL, "\0");
      tmp->gpa = atof(data);
      tmp->next = department;
      department = tmp;

    }
  }else{
    printf("Error while opening the file\n");
  }
  printf("End of Phase 1 for the admissions office\n");
  fclose(file);


  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
  printf("The admissions office has TCP port %s and IP address %s\n", PORT, s);
  freeaddrinfo(servinfo);

  for(int ii = 0; ii < NO_STUDENTS; ii++){
    sin_size = sizeof(their_addr);
    if((child = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1){
      perror("accept\n");
      continue;
    }
    if(!fork()){
      close(sockfd);

      int numBytes;
      char buff[12];
      char * data;
      int choice = 0;

      //Student student;
      struct Student{
        double grade;
        std::string interest[3];
      }student;

      if((numBytes = recv(child, buff, 7, 0)) == -1){
        exit(1);
      }
      buff[numBytes] = '\0';
      data = strtok(buff, ":");
      data = strtok(NULL, "\0");
      student.grade = atof(data);

      numBytes = recv(child, buff, 12, 0);
      while(numBytes != 8){
        if(numBytes == -1){
          exit(1);
        }
        buff[numBytes] ='\0';
        data = strtok(buff, ":");
        data = strtok(NULL, "\0");
        student.interest[choice++] = data;
        numBytes = recv(child, buff, 12, 0);
      }
      printf("Admissions office received application from Student<%d>\n", ii+1);
      //printf("Student<%d> grade: %f\n", ii + 1, student.grade);
      bool flags[] = {false, false};
      struct Node * tmp = NULL;
      for(int jj = 0; jj < choice; jj++){
        for(tmp = department; tmp != NULL; tmp = tmp->next){
          if(!strcmp(student.interest[jj].c_str(), tmp->dept)){
            flags[1] = true;
            if(student.grade >= tmp->gpa){
              flags[0] = true;
            }
            break;
          }
        }
        if(flags[0]){
          break;
        }
      }
      if(tmp != NULL){
        char stuFile[20];
        strcpy(stuFile, student_out[ii].c_str());
        char depFile[20];
        switch(tmp->dept[0]){
          case 'A':
            strcpy(depFile, "deptA_prog.txt");
            break;
          case 'B':
            strcpy(depFile, "deptB_prog.txt");
            break;
          default:
            strcpy(depFile, "deptC_prog.txt");
        }
        FILE *file;
        file = fopen(stuFile, "a");
        if(file != NULL){
          if(flags[0]){
            fprintf(file, "Accept#%s#department%c\n", tmp->dept, tmp->dept[0]);
          }
          else{
            fprintf(file, "Reject");
          }
        }
        else{
          printf("Error in writing file: Student\n");
        }
        fclose(file);

        //FILE *file;
        file = fopen(depFile, "a");
        if(file != NULL){
          if(flags[0]){
            fprintf(file, "Student%d#%2.1f#%s\n", ii + 1, student.grade, tmp->dept);
          }
        }
        else{
          printf("Error in writing file: Department\n");
        }
        fclose(file);
      }
      if(!flags[1]){
        if(send(child, "0", 2, 0) == -1){
          perror("send\n");
        }
      }
      else{
        if(send(child, "1", 2, 0) == -1){
          perror("send\n");
        }
      }
      close(child);
      exit(0);
    }
    close(child);
  }

  int _status1, _status2, _status3, _status4, _status5;
  wait(&_status1);
  wait(&_status2);
  wait(&_status3);
  wait(&_status4);
  wait(&_status5);

  free(department);

  for(int ii = 0; ii < NO_STUDENTS; ii++){
    if(fork() == 0){
      char files[20];
      char port[20];

      strcpy(files, student_out[ii].c_str());
      strcpy(port, student_ports[ii].c_str());

      //Beej's UDP setup
      memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_DGRAM;

      if((rv = getaddrinfo(HOST, port, &hints, &servinfo))){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
      }

      for(p = servinfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
          close(sockfd);
          perror("admissions: socket");
          continue;
        }
        break;
      }
      if(p == NULL){
        fprintf(stderr, "admissions: failed to bind socket\n");
        return 1;
      }

      file = fopen(files, "r");
      if(file != NULL){
        int numBytes;
        char pkt[22];
        fgets(pkt, 22, file);
        if((numBytes = sendto(sockfd, pkt, sizeof(pkt), 0, p->ai_addr, p->ai_addrlen)) == -1){
          perror("admissions: sendto");
          exit(1);
        }

        struct sockaddr_in my_addr;
        socklen_t len;
        len = sizeof(my_addr);
        getsockname(sockfd, (struct sockaddr *)&my_addr, &len);
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
        printf("The admission office has UDP port %d and IP address %s for Phase2\n", (int)ntohs(my_addr.sin_port), s);
        printf("The admissions office has sent the application result to Student<%d>\n", ii + 1);
      }
      else{
        printf("Error while opening the file.\n");
      }
      fclose(file);
      freeaddrinfo(servinfo);
      close(sockfd);
      exit(0);
    }
    close(sockfd);
  }
  int status1_i, status2_i, status3_i, status4_i, status5_i;
  wait(&status1_i);
  wait(&status2_i);
  wait(&status3_i);
  wait(&status4_i);
  wait(&status5_i);

  for(int ii = 0; ii < NO_DEPARTMENTS; ii++){
    if(!fork()){
      char files[20];
      char port[10];
      char dept[15];
      strcpy(files, department_out[ii].c_str());
      strcpy(port, department_ports[ii].c_str());
      strcpy(dept, department_names[ii].c_str());

      memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_DGRAM;

      if((rv = getaddrinfo(HOST, port, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
      }

      for(p = servinfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
          close(sockfd);
          perror("admissions: socket");
          continue;
        }
        break;
      }
      if(p == NULL){
        fprintf(stderr, "admissions: failed to bind socket\n");
        return 1;
      }

      file = fopen(files, "r");
      if(file != NULL){
        int numBytes;
        char pkt[20];
        while(fgets(pkt, 20, file)){
          char * data = strtok(pkt, "\n");
          if((numBytes = sendto(sockfd, data, (int)strlen(data), 0, p->ai_addr, p->ai_addrlen)) == -1){
            perror("admissions: sendto\n");
            exit(1);
          }
        }
        char _exit[] = "finished";
        if((numBytes = sendto(sockfd, _exit,(int)strlen(_exit), 0, p->ai_addr, p->ai_addrlen)) == -1){
          perror("admissions: sendto");
          exit(1);
        }

        struct sockaddr_in my_addr;
        socklen_t len;
        len = sizeof(my_addr);
        getsockname(sockfd, (struct sockaddr *)&my_addr, &len);
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
        printf("The admission office has UDP port %d and IP address %s for Phase2\n", (int)ntohs(my_addr.sin_port), s);
        printf("The admissions office has sent the application result to <%s>\n", dept);
      }
      else{
        printf("Error in opening file\n");
      }
      freeaddrinfo(servinfo);
      fclose(file);
      close(sockfd);
      exit(0);
    }
    close(sockfd);
  }
  int status1_ii, status2_ii, status3_ii;
  wait(&status1_ii);
  wait(&status2_ii);
  wait(&status3_ii);

  printf("End of Phase2 for the admissions office\n");
  return 0;
}
