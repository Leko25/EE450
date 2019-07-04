//Name: Kelechi Ogudu
//USC_ID: 5413285132
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define LINE_SIZE 14

struct Node{
  char * info;
  struct Node * next;
};

//Append elements to list -- O(n)
void _append(struct Node ** list, char * p){
   struct Node * newNode = (struct Node *)malloc(sizeof(struct Node));
   newNode->info = (char *)malloc(strlen(p) + 1); //talk to Xuan about this for grader -- previous malloc(strlen(p) + 1)
   strcpy(newNode->info, p);
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
    printf("%s", tmp->info);
    tmp = tmp->next;
    printf("\n");
  }
}

//Reads file and creates a linked list with its contents
void _readFile(struct Node ** list){
  FILE *file;
  char ch;
  file = fopen("student4.txt", "r"); //read file
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
    data[ii_data] = '\0';
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
  struct Node * list = NULL;
  _readFile(&(list));
  _printList(list);
  return 0;
}
