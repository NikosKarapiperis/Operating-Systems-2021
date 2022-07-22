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

//Function για να τυπωσουμε το ονομα του δικου μας φλοιου
void name(void){
   printf("\n\n Hello,this is my shell.\n\n");
   printf("\nThe name of my shell is mysh\n\n");
}

void user_and_dir(void){
    //Παιρνουμε το ονομα του user ωστε να το εμφανισουμε στη συνεχεια
        char *buf;
        buf = (char *)malloc(10*sizeof(char));
        if(buf==NULL){
         perror("malloc");
        }

        buf = getlogin();
        //Παιρνουμε το current directory
        char cwd[PATH_MAX];
        if(getcwd(cwd,sizeof(cwd)) == NULL){
        perror("getcwd() error");
        }
        //Tυπωνουμε το ονομα του user και current directory
        printf("%s:%s> \n",buf,cwd);
}

//Συναρτηση για να τσεκαρουμε ποιο θα ειναι το exit του child
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

//function για να διαχειριστουμε τα σηματα οπως εμεις θελουμε
void signalhandler(int sign){
   char x;
   printf("Caught signal %d\n",sign);

   printf("Stop programm for sure;");
   //ο χρηστης δινει την επιλογη του μεσω της scanf
   scanf("%s",&x);
   //y:Αν ο χρηστης θελει να τερματισει το προγραμμα, n:αν θελει να συνεχισει
      if(x=='y'){
        exit(sign);
        }else if(x=='n'){
          signal(SIGINT,SIG_IGN);
          signal(SIGTERM,SIG_IGN);
        }
}

//Function mygrep για να ελεγχουμε αν υπαρχει το string που εισαγαμε στο συγκεκριμενο file
//Αν υπαρχει τυπωνουμε την γραμμη στην οποια βρεθηκε


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

        m = strlen(string); // Παιρνουμε το μεγεθος του string
        
        line = 0;
         // Με την fgets παιρναμε στην ουσια ολα τα δεδομενα του fp
        //στον πινακα απο χαρακτηρες μεγεθους 256 buffer
        //Η διαδικασια συνεχιζεται εως ουτε η fgets δεν ισουται με NULL

        while (fgets(buffer, 256, fp) != NULL) {

                i = 0;
                n = strlen(buffer);
                // Το συγκεκριμενο loop βρισκει αν υπαρχει το string στην συγκεκριμενη γραμμη του file
                // Τυπωνουμε την καθε γραμμη στην οποια βρεθηκε
                // Στην ουσια το loop διαβαζει καθε λεξη του file και την συγκρινει με το input string
                while (i < n) {

                        //Συγκρινουμε την λεξη που εγινε input με την current λεξη
                        j = 0;
                        while (i < n && j < m && buffer[i] == string[j]) {
                                ++i, ++j;
                        }

                        // Αν ειναι ιδιες οι λεξεις τυπωνουμε την γραμμη στην οποια βρεθηκε
                        if ((i == n || buffer[i] == ' ' || buffer[i] == '\n') && j == m) {
                                printf("Line: %d ", line);

                        }

                          // Προχωραμε στην επομενη λεξη του file
                        while (i < n && buffer[i] != ' ') {
                                ++i;
                      }

                //Παμε στην επομενη γραμμη του file
               //Kρατωντας αυτη την μεταβλητη , θα ξερουμε και ποσα threads πρεπει να χρησιμοποιησουμε(1 thread ανα γραμμη)
                ++line;

        }
       //Κλεισιμο του file

        fclose(fp);
}

}




int main(int argc){
   signal(SIGINT,signalhandler);
   signal(SIGTERM,signalhandler);


   //function για να τυπωσει το ονομα του myshell
   name();

   //function for printing user and current directory
   user_and_dir();

  //loop while ωστε να συνεχιζεται η διαδικασια και να εμφανιζεται το prompt
  //στον χρηστη μεχρις οτου ερθει καποιο σημα και ο χρηστης αποφασισει να
  //τερματισει το προγραμμα
   while(1){
   pid_t pid;
   int status;
   time_t when;
   char *argv[MAXCOMM];
   char input;

   time(&when);

   printf("THIS IS MY PROMPT\n");

   //Πινακας για την αποθηκευση του input απο τον user
   char command[128]={0};

   //Παιρνουμε τo input του user μεσω της fgets
   fgets(command,128,stdin);

   command[strlen(command)-1]='\0';

   //Διαδικασια για tokenization ωστε ο χρηστης να μπορει να δωσει και
   //Παραμετρους στις εντολες

   //Πηρα την ιδεα του TOKENIZE απο το stackoverflow
   //Και το διαχειριστηκα καταλληλα

   //Με την function strtok υποστηριζουμε και κενα ωστε να μπορουμε να
  //Εκτελεσουμε πχ την εντολη ls με παραμετρο -l

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
 //Γραφουμε μεσα στο αρχειο μεσω της write την ωρα και το input του user
  write(fd,ctime(&when),strlen(ctime(&when)));
  write(fd,"\n",2);
  write(fd,command,128);
  write(fd,"\n",2);

   close(fd);

   pid=fork();  //Φτιαχνουμε νεα διεργασια μεσω της fork

   if(pid!=0)  //Εκτελειται ο πατερας γιατι η fork επιστρεφει 0 στο child
   {
        if(wait(&status)==-1) //Περιμενουμε μεσω της wait μεχρι να τελειωσει το
                            //child
        {
        perror("Wait"); //Αν επιστρεψει -1 η wait απετυχε
        }
        check_child_exit(status);
        }
        else{   //the child process
        execvp(command_tok[0],command_tok); //Execute την εντολη που εδωσε ο user
        perror("execvp");
        exit(EXIT_FAILURE);  //Aν η exec απετυχε τυπωνει καταλληλο μηνυμα και
                            //Κανει EXIT_FAILURE
              }

}

    return 0;


}


