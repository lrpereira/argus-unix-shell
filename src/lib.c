#include "argus.h"

void help_daemon(int argc) {
  if (argc != 1) {
    fprintf(stderr, "No args needed.\nExiting...\n");
    exit(-1);
  }
}

void create_channel(char *channel) {
  if (mkfifo(channel, 0666) != 0) {

    fprintf(stderr, "%s: %s.\n", channel, strerror(errno));

    /* errno for file exists is 2 */
    if (strcmp(strerror(errno), "File exists")!=0)
      exit(-1);
  }
}

void help_client() {
  printf("Help client\n");
}

void send_message(int fd, int token, char *message) {
  char msg[BUFFERSIZE];
  sprintf(msg, "%d-%s", token, message);
  write(fd, msg, strlen(msg)+1);
  /* perror("Writing"); */
}

void receive_message(int fd, char* message) {
  read(fd, message, 1024);
  /* printf("message: %s\n", message); */
  /* perror("Reading"); */
}

void clean_command(char *command) {

  if (command[0]==' ') {
    for (int i=0; i<(int)strlen(command); ++i)
      command[i]=command[i+1];
  }
  
  int size = strlen(command);
  
  if (command[size-1]==' ')
    command[size-1]='\0';
}

char** parse_command(char *command) {
  int i=0;

  char* command_aux=strdup(command);
  char** aux=malloc(10 * sizeof(char*));
  
  aux[i]=strsep(&command_aux," ");
  while (aux[i]!=NULL) {
    //printf("%s\n", aux[i]);
    aux[++i]=strsep(&command_aux," ");
  }

  /* for (i=0; aux[i]!=NULL; ++i) { */
  /*   printf("%s\n",aux[i]); */
  /* } */

  return aux;
}
