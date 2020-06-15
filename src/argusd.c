#include "argus.h"

void parse_message(char *buffer, int *task_opt_ptr, int* task_pipes_ptr, char* parsed[10]) {

  int i=0;
  char* unparsed = strdup(buffer);
  (*task_opt_ptr)=atoi(strsep(&unparsed, "-"));

  parsed[i]=strsep(&unparsed,"|");
  while (parsed[i]!=NULL)
    parsed[++i]=strsep(&unparsed,"|");

  (*task_pipes_ptr)=i-1;
}

void exec_task(char *task_parsed[10], int task_pipes, char* channel_output) {

  int i=0, j=0, status, fd_out;
  int first_cmd = TRUE, last_cmd = FALSE;
  pid_t pid;

  int pipes[2*task_pipes];

  for (i=0; i<task_pipes; i++ ) {
    if (pipe(pipes + i*2) < 0 ){
      perror("Pipe"); exit(EXIT_FAILURE);
    }
  }

  i=0;
  /* task_parsed[i]!=NULL */
  while (i<task_pipes+1) {

    pid = fork();
    if (pid==0) {

      clean_command(task_parsed[i]);
      char** run = parse_command(task_parsed[i]);

      /* If not first command */
      if (first_cmd == FALSE) {

        /* If not first comment and is last comment */
        if (last_cmd == TRUE) {
          fd_out = open(channel_output, O_WRONLY);

          /* Redirect output of execvp to FIFO */
          if (dup2(fd_out, 1)<0) {perror("1st Dup2 Last"); exit(EXIT_FAILURE);}

          /* Redirect standard input to read end of pipe */
          if (dup2(pipes[(i-1)*2], 0)<0) {perror("1st Dup2 Last"); exit(EXIT_FAILURE);}

          close(fd_out);
        }

        /* If not first and not last command */
        else {
          if (dup2(pipes[(i-1)*2], 0)<0) {perror("1st Dup2"); exit(EXIT_FAILURE);}
        }
      }

      /* If not last command */
      if (last_cmd == FALSE) {
        if (dup2(pipes[(i*2)+1], 1)<0) {perror("2nd Dup2"); exit(EXIT_FAILURE);}
      }

      /* Close all pipe fds */
      for (j=0; j<2*task_pipes; ++j) close(pipes[j]);

      execvp(run[0], run);
      perror("Execvp");
    }

    else if (pid<0) {
      perror("Forking");
      exit(EXIT_FAILURE);
    }

    /* Parent pid controling execution */
    if (first_cmd==TRUE) first_cmd = FALSE;
    if (i+1==task_pipes) last_cmd = TRUE;
    i++;
  }

  /* Parent pid closes all pipes and waits for childs */
  for (j=0; j<2*task_pipes; ++j) close(pipes[j]);
  for (j=0; j<2*task_pipes; ++j) wait(&status);
}

int main(int argc, char *argv[])
{
  UNUSED(argv);

  help_daemon(argc);

  int fd_in;

  int task_opt, task_pipes;
  int* task_opt_ptr=&task_opt;
  int* task_pipes_ptr=&task_pipes;

  char buffer[BUFFERSIZE];
  char* task_parsed[10];

  char* channel_input  = "./bin/channel_input";
  char* channel_output = "./bin/channel_output";

  create_channel(channel_input);
  create_channel(channel_output);

  while (1) {

    printf("Waiting for client...\n");
    fd_in  = open(channel_input, O_RDONLY);
    printf("Client connected.\n\n");

    receive_message(fd_in, buffer);
    printf("Server msg received: %s\n\n", buffer);
    close(fd_in);

    parse_message(buffer, task_opt_ptr, task_pipes_ptr, task_parsed);

    /* printf("Task opt: %d\nTask num pipes: %d\n", task_opt, task_pipes); */
    /* int i=0; */
    /* while(parsed[i]!=NULL) */
    /*   printf("%s\n", parsed[i++]); */

    switch (task_opt) {
    case 900:
      exec_task(task_parsed, task_pipes, channel_output);
      break;
    case 901:
      break;
    case 902:
      break;
    case 903:
      break;
    case 904:
      break;
    case 905:
      break;
    case 906:
      break;
    default:
      printf("Error parsing option.\n");
      break;
    }

    /* fd_out = open(channel_output, O_WRONLY); */
    /* send_message(fd_out, 1, buffer); */
    /* printf("\nServer msg sent: %s.\n\n", buffer); */
    /* close(fd_out); */

    buffer[0]='\0';
  }

  exit(EXIT_SUCCESS);
}
