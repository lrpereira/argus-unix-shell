/**
   NOTES:
   Atoi returns 0 in case of conversion error.

   OPTIONS CODE:
   900 -> exec task (-e OR executar "...")
   901 -> set inactivity time limit (-i n OR tempo-inatividade n)
   902 -> set task execution time limit (-m n OR tempo-execução n)
   903 -> list tasks (-l OR listar)
   904 -> end task (-t n OR terminar n)
   905 -> history (-r OR historico)
   906 -> help (-h OR ajuda)
 */

#include "argus.h"
#include <stdio.h>

void list_running_execs() { printf("List_running_execs\n"); }
void history() { printf("History\n"); }

void set_inactivity_timelimit() { printf("Set inactivity time\n"); }
void set_execution_timelimit() { printf("Set execution time\n"); }
void end_task_n() { printf("End task execution\n"); }

void run_task(char* argv2) {

  int fd_in;
  int fd_out;

  char* channel_input  = "./bin/channel_input";
  char* channel_output = "./bin/channel_output";

  char* unparsed=strdup(argv2);
  char message[BUFFERSIZE];

  fd_in = open(channel_input, O_WRONLY);
  printf("Client connected.\n");
  send_message(fd_in, 900, unparsed);
  printf("Client msg sent: %s\n", unparsed);
  close(fd_in);

  printf("Waiting to read\n");
  fd_out = open(channel_output, O_RDONLY);
  printf("Starting to read\n");
  receive_message(fd_out, message);
  printf("Client msg received: %s\n", message);
  close(fd_out);
}

void cli_mode() { printf("CLI mode.\n"); }

int main(int argc, char *argv[])
{
  /* Aux for options -i and -m */
  int n;

  if (argc==1) cli_mode();

  if (argc==2) {
    if (strcmp(argv[1], "-h")==0) help_client();
    else if (strcmp(argv[1], "-l")==0) list_running_execs();
    else if (strcmp(argv[1], "-r")==0) history();
    else {
      fprintf(stderr, "Unkown option.\nExiting...\n");
      exit(1);
    }
  }

  if (argc==3) {
    if (strcmp(argv[1], "-i")==0) {
      n=atoi(argv[2]);
      if (n>0) set_inactivity_timelimit();
      else {
        fprintf(stderr, "Invalid number of seconds.\nExiting...\n");
        exit(1);
      }
    }
    else if (strcmp(argv[1], "-m")==0) {
      n=atoi(argv[2]);
      if (n>0) set_execution_timelimit();
      else {
        fprintf(stderr, "Invalid number of seconds.\nExiting...\n");
        exit(1);
      }
    }
    else if (strcmp(argv[1], "-t")==0) {
      n=atoi(argv[2]);
      if (n>0) end_task_n();
      else {
        fprintf(stderr, "Invalid task number.\nExiting...\n");
        exit(1);
      }
    }
    else if (strcmp(argv[1], "-e")==0) {
      if (strlen(argv[2])>0) run_task(argv[2]);
      else {
        fprintf(stderr, "Invalid string.\nExiting...\n");
        exit(1);
      }
    }
    else {
      fprintf(stderr, "Unkown option.\nExiting...\n");
      exit(1);
    }
  }

  if (argc>=4) {
    fprintf(stderr, "Wrong use of Argus.\n\n");
    help_client();
    exit(1);
  }

  exit(0);
}
