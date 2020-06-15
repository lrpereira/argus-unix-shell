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

void set_inactivity_timelimit(char* buffer) {
  char* channel_input  = "./bin/channel_input";
  char* channel_output = "./bin/channel_output";

  char* unparsed=strdup(buffer);
  char message[BUFFERSIZE];

  send_message(channel_input, 901, unparsed);
  receive_message(channel_output, message);
}

void set_execution_timelimit(char* buffer) {
  char* channel_input  = "./bin/channel_input";
  char* channel_output = "./bin/channel_output";

  char* unparsed=strdup(buffer);
  char message[BUFFERSIZE];

  send_message(channel_input, 902, unparsed);
  receive_message(channel_output, message);
}

void history() { printf("History\n"); }
void list_running_execs() { printf("List_running_execs\n"); }
void end_task_n() { printf("End task execution\n"); }

void run_task(char* argv2) {

  char* channel_input  = "./bin/channel_input";
  char* channel_output = "./bin/channel_output";

  char* unparsed=strdup(argv2);
  char message[BUFFERSIZE];

  send_message(channel_input, 900, unparsed);
  receive_message(channel_output, message);
}

void cli_mode() {

  char* buffer=(char*)malloc(BUFFERSIZE * sizeof(char));
  char* token=(char*)malloc(20 * sizeof(char));

  printf("argus$ "); fflush(stdout);

  while (read(STDIN, buffer, BUFFERSIZE) != 0) {

    /* Set \0 in \n */
    set_string_end(buffer);

    token=strsep(&buffer, " ");

    if (strcmp(token, "executar")==0) {
      clean_quotes(buffer);
      run_task(buffer);
    }
    else if (strcmp(token, "tempo-inatividade")==0) {
      set_inactivity_timelimit(buffer);
    }
    else if (strcmp(token, "tempo-execução")==0) {
      set_execution_timelimit(buffer);
    }
    else if (strcmp(token, "listar")==0) {
    }
    else if (strcmp(token, "terminar")==0) {
    }
    else if (strcmp(token, "historico")==0) {
    }
    else if (strcmp(token, "ajuda")==0) {
    }
    else {
      printf("Comando não reconhecido.\n");
      help_client();
    }

    printf("\nargus$ "); fflush(stdout);
  }
}

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
    if (strcmp(argv[1], "-e")==0) {
      if (strlen(argv[2])>0) run_task(argv[2]);
      else {
        fprintf(stderr, "Invalid string.\nExiting...\n");
        exit(1);
      }
    }

    else if (strcmp(argv[1], "-i")==0) {
      n=atoi(argv[2]);
      if (n>0) set_inactivity_timelimit(argv[2]);
      else {
        fprintf(stderr, "Invalid number of seconds.\nExiting...\n");
        exit(1);
      }
    }

    else if (strcmp(argv[1], "-m")==0) {
      n=atoi(argv[2]);
      if (n>0) set_execution_timelimit(argv[2]);
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
