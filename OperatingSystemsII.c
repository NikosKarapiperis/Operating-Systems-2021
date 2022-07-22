#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <pthread.h>

#define MAXCOMM 1000 //this is max number of commands

//Function ��� �� ��������� �� ����� ��� ����� ��� ������
void name(void){
   printf("\n\n Hello,this is my shell.\n\n");
   printf("\nThe name of my shell is mysh\n\n");
}

void user_and_dir(void){
    //��������� �� ����� ��� user ���� �� �� ����������� ��� ��������
        char *buf;
        buf = (char *)malloc(10*sizeof(char));
        if(buf==NULL){
         perror("malloc");
        }

        buf = getlogin();
        //��������� �� current directory
        char cwd[PATH_MAX];
        if(getcwd(cwd,sizeof(cwd)) == NULL){
        perror("getcwd() error");
        }
        //T�������� �� ����� ��� user ��� current directory
        printf("%s:%s> \n",buf,cwd);
}

//��������� ��� �� ���������� ���� �� ����� �� exit ��� child
void check_child_exit(int status){
   if(WIFEXITED(status)){
        printf("Child ended normally, Exit code is %d\n", WEXITSTATUS(status));
        }
   else if(WIFSIGNALED(status)){
        printf("Child ended because of an uncaught signal,Signal =%d\n",WTERMSIG(status));
        }
   else if(WIFSTOPPED(status)){
        printf("Child process has stopped ,Signal = %d\n",WSTOPSIG(status));
        }
}

//function ��� �� �������������� �� ������ ���� ����� �������
void signalhandler(int sign){
   char x;
   printf("Caught signal %d\n",sign);

   printf("Stop programm for sure;");
   //� ������� ����� ��� ������� ��� ���� ��� scanf
   scanf("%s",&x);
   //y:�� � ������� ����� �� ���������� �� ���������, n:�� ����� �� ���������
      if(x=='y'){
        exit(sign);
        }else if(x=='n'){
          signal(SIGINT,SIG_IGN);
          signal(SIGTERM,SIG_IGN);
        }
}

//Function mygrep ��� �� ��������� �� ������� �� string ��� �������� ��� ������������ file
//�� ������� ��������� ��� ������ ���� ����� �������


void mygrep(char *file , char *string){
   char buffer[256];
        int n, m, i, j, line;

        FILE* fp;
        fp = fopen(file, "r");  // open file
        if(fp==NULL){
          perror("fopen");
        }

        printf("Enter the word you want to search in the file: ");
        scanf("%s",string);

        m = strlen(string); // ��������� �� ������� ��� string
        
        line = 0;
         // �� ��� fgets �������� ���� ����� ��� �� �������� ��� fp
        //���� ������ ��� ���������� �������� 256 buffer
        //� ���������� ����������� ��� ���� � fgets ��� ������� �� NULL

        while (fgets(buffer, 256, fp) != NULL) {

                i = 0;
                n = strlen(buffer);
                // �� ������������ loop ������� �� ������� �� string ���� ������������ ������ ��� file
                // ��������� ��� ���� ������ ���� ����� �������
                // ���� ����� �� loop �������� ���� ���� ��� file ��� ��� ��������� �� �� input string
                while (i < n) {

                        //����������� ��� ���� ��� ����� input �� ��� current ����
                        j = 0;
                        while (i < n && j < m && buffer[i] == string[j]) {
                                ++i, ++j;
                        }

                        // �� ����� ����� �� ������ ��������� ��� ������ ���� ����� �������
                        if ((i == n || buffer[i] == ' ' || buffer[i] == '\n') && j == m) {
                                printf("Line: %d ", line);

                        }

                          // ��������� ���� ������� ���� ��� file
                        while (i < n && buffer[i] != ' ') {
                                ++i;
                      }

                //���� ���� ������� ������ ��� file
               //K�������� ���� ��� ��������� , �� ������� ��� ���� threads ������ �� ����������������(1 thread ��� ������)
                ++line;

        }
       //�������� ��� file

        fclose(fp);
}

}




int main(int argc){
   signal(SIGINT,signalhandler);
   signal(SIGTERM,signalhandler);


   //function ��� �� ������� �� ����� ��� myshell
   name();

   //function for printing user and current directory
   user_and_dir();

  //loop while ���� �� ����������� � ���������� ��� �� ����������� �� prompt
  //���� ������ ������ ���� ����� ������ ���� ��� � ������� ���������� ��
  //���������� �� ���������
   while(1){
   pid_t pid;
   int status;
   time_t when;
   char *argv[MAXCOMM];
   char input;

   time(&when);

   printf("THIS IS MY PROMPT\n");

   //������� ��� ��� ���������� ��� input ��� ��� user
   char command[128]={0};

   //��������� �o input ��� user ���� ��� fgets
   fgets(command,128,stdin);

   command[strlen(command)-1]='\0';

   //���������� ��� tokenization ���� � ������� �� ������ �� ����� ���
   //����������� ���� �������

   //���� ��� ���� ��� TOKENIZE ��� �� stackoverflow
   //��� �� ������������� ���������

   //�� ��� function strtok ������������� ��� ���� ���� �� �������� ��
  //����������� �� ��� ������ ls �� ��������� -l

   char *p = strtok(command, " ");
   char *command_tok[128];
   int i=0;
   while(p){
        command_tok[i++]=p;
        p = strtok(NULL, " ");
   }
   command_tok[i] = NULL;



   //open file for write and create
   int fd;
   fd = open("data.log",O_WRONLY | O_CREAT |O_APPEND,0600);
   if(fd==-1){
        perror("open");
        exit(1);
  }
 //�������� ���� ��� ������ ���� ��� write ��� ��� ��� �� input ��� user
  write(fd,ctime(&when),strlen(ctime(&when)));
  write(fd,"\n",2);
  write(fd,command,128);
  write(fd,"\n",2);

   close(fd);

   pid=fork();  //���������� ��� ��������� ���� ��� fork

   if(pid!=0)  //���������� � ������� ����� � fork ���������� 0 ��� child
   {
        if(wait(&status)==-1) //����������� ���� ��� wait ����� �� ��������� ��
                            //child
        {
        perror("Wait"); //�� ���������� -1 � wait �������
        }
        check_child_exit(status);
        }
        else{   //the child process
        execvp(command_tok[0],command_tok); //Execute ��� ������ ��� ����� � user
        perror("execvp");
        exit(EXIT_FAILURE);  //A� � exec ������� ������� ��������� ������ ���
                            //����� EXIT_FAILURE
              }

}

    return 0;


}


