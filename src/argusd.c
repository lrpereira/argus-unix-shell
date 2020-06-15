#include "argus.h"

int timeout = 0;

void alarm_handler() {timeout = 1;}

int inactivity_timeout = 0;
int execution_timeout = 0;

int command_counter = 0;

void set_inactivity_timeout(char *buffer, char* channel) {
  char* message = "Inactivity timeout set";
  /* inactivity_timeout=atoi(buffer); */
  execution_timeout=atoi(buffer);
  command_counter++;
  send_message(channel, command_counter, message);
}

void set_execution_timeout(char *buffer, char* channel) {
  char* message = "Execution timeout set";
  execution_timeout=atoi(buffer);
  command_counter++;
  send_message(channel, command_counter, message);
}

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
  int first_cmd = TRUE, last_cmd = FALSE, single_cmd = FALSE;
  pid_t pid;
  pid_t childs[task_pipes+1];

  if (task_pipes==0) single_cmd = TRUE;

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

      /* printf("\nChild => I: %d PID: %d, Cmd: %s\n", i, getpid(), task_parsed[i]); fflush(stdout); */

      /* There is only 1 command to execvp */
      if (single_cmd == TRUE) {

        /* Open server side fifo write end */
        fd_out = open(channel_output, O_WRONLY);

        /* Redirect output of execvp to FIFO */
        if (dup2(fd_out, 1)<0) {perror("Dup2 Single"); exit(EXIT_FAILURE);}

        /* Close fifo */
        close(fd_out);
      }

      /* There is two or more commands to execvp */
      else if (single_cmd == FALSE) {

        /* If not first command */
        if (first_cmd == FALSE) {

          /* If not first command and is last comment */
          if (last_cmd == TRUE) {

            /* Open server side fifo write end */
            fd_out = open(channel_output, O_WRONLY);

            /* Redirect execvp output to FIFO */
            if (dup2(fd_out, 1)<0) {perror("Dup2 Last FIFO"); exit(EXIT_FAILURE);}

            /* Redirects execvp input from stdin to read end of pipe */
            if (dup2(pipes[(i-1)*2], 0)<0) {perror("Dup2 Last"); exit(EXIT_FAILURE);}

            /* Close fifo */
            close(fd_out);
          }

          /* If not first and not last command */
          else {

            /* Redirects execvp input from stdin to read end of pipe */
            if (dup2(pipes[(i-1)*2], 0)<0) {perror("Dup2 not fst and not last"); exit(EXIT_FAILURE);}
          }
        }

        /* If not last command */
        if (last_cmd == FALSE) {

          /* Redirect execvp output to next execvp input */
          if (dup2(pipes[(i*2)+1], 1)<0) {perror("Dup2 not last"); exit(EXIT_FAILURE);}
        }
      }

      /* Close all pipe fds */
      for (j=0; j<2*task_pipes; ++j) close(pipes[j]);

      /* exit(0); */
      execvp(run[0], run);
      perror("Execvp");
    }

    else if (pid<0) {
      perror("Forking");
      exit(EXIT_FAILURE);
    }

    /* Parent controling execution */
    childs[i]=pid;
    /* printf("Parent pid:%d Child pid:%d\n", getppid(), childs[i]); */
    if (first_cmd==TRUE) first_cmd = FALSE;
    if (i+1==task_pipes) last_cmd = TRUE;
    i++;
  }

  /* Parent pid closes all pipes */
  for (j=0; j<2*task_pipes; ++j)
    close(pipes[j]);

  /* If execution_timeout equals 0 then no alarm is set */
  if (execution_timeout) {
    signal(SIGALRM, alarm_handler);
    alarm(execution_timeout);
    pause();
    alarm(0);
  }

  if (timeout) {
    /* Pequeno bug aqui */
    /* char* message="Timeout."; */
    /* send_message(channel_output, command_counter, message); */
    /**
       WNOHANG causes the call to waitpid to return status information
       immediately without waiting for the specified process to terminate
    */
    int result[i];
    for (j=0; j<i; ++j) {
      result[j] = waitpid(childs[j], NULL, WNOHANG);
      if (result[j]==0) {
        printf("Killing pid:%d\n", childs[j]); fflush(stdout);
        kill(childs[j], SIGKILL);
      }
      else {
        alarm(0);
      }
    }
  }

  for (j=0; j<task_pipes+1; ++j) wait(&status);
}

/**
   SERVER ENTRY POINT
 */
int main(int argc, char *argv[])
{
  UNUSED(argv);

  help_daemon(argc);

  int task_opt, task_pipes;
  int* task_opt_ptr=&task_opt;
  int* task_pipes_ptr=&task_pipes;

  char buffer[BUFFERSIZE];
  char* task_parsed[10];

  char* channel_input  = "channel_input";
  char* channel_output = "channel_output";

  create_channel(channel_input);
  create_channel(channel_output);

  while (1) {

    printf("\nWaiting for client...\n");
    receive_message(channel_input, buffer);
    printf("Client connected.\n");

    parse_message(buffer, task_opt_ptr, task_pipes_ptr, task_parsed);

    /* printf("Task opt: %d\nTask num pipes: %d\n", task_opt, task_pipes); */
    /* int i=0; */
    /* while(task_parsed[i]!=NULL) */
    /*   printf("%s\n", task_parsed[i++]); */

    switch (task_opt) {
    case 900:
      exec_task(task_parsed, task_pipes, channel_output);
      timeout = 0;
      break;
    case 901:
      set_inactivity_timeout(task_parsed[0], channel_output);
      break;
    case 902:
      set_execution_timeout(task_parsed[0], channel_output);
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

    buffer[0]='\0';
  }

  exit(EXIT_SUCCESS);
}
