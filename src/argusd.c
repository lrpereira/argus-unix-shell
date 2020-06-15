#include "argus.h"
#include <stdio.h>
#include <unistd.h>

int timeout = 0;
int childs_done = 0;

void child_handler() {childs_done++;}
void alarm_handler() {timeout = 1;}

int inactivity_timeout = 0;
int execution_timeout = 0;

int command_counter = 0;

void set_inactivity_timeout(char *buffer, char* channel) {
  char* message = "Inactivity timeout set";
  inactivity_timeout=atoi(buffer);
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

      printf("\nChild => I: %d PID: %d, Cmd: %s\n", i, getpid(), task_parsed[i]); fflush(stdout);

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
    printf("CHILD PID:%d\n",childs[i]); fflush(stdout);

    if (first_cmd==TRUE) first_cmd = FALSE;
    if (i+1==task_pipes) last_cmd = TRUE;
    i++;
  }

  /* Parent pid closes all pipes */
  for (j=0; j<2*task_pipes; ++j)
    close(pipes[j]);

  /* If execution_timeout equals 0 then no alarm is set */
  signal(SIGALRM, alarm_handler);
  signal(SIGCHLD, child_handler);

  alarm(execution_timeout);

  if (single_cmd == TRUE) {
    pause();

    if (timeout) {
      printf("Task execution timed out.\n"); fflush(stdout);
      int result = waitpid(pid, NULL, WNOHANG);
      if (result == 0) {
        /* Kill child pid */
        kill(pid, 9);
        wait(&status);
      }
    }
    else if (childs_done) {
      printf("Execvp finished normally.\n"); fflush(stdout);
      wait(&status);
    }
  }
  else {
    /* for (j=0; j<task_pipes+1; ++j) */
    pause();

    if (timeout) {
      printf("Task execution timed out.\n"); fflush(stdout);
      int result[task_pipes+1];
      for (j=0; j<task_pipes+1; j++) {
        result[i]=waitpid(pid, NULL, WNOHANG);
        if (result[i] == 0) {
          /* Kill child pid */
          kill(pid, 9);
          wait(&status);
        }
      }
    }
    else if (childs_done==task_pipes+1) {
      printf("Entrou.\n"); fflush(stdout);
      for (j=0; j<task_pipes+1; ++j) {
        wait(&status);
        printf("Execvp finished normally:%d.\n", j); fflush(stdout);
      }
    }
  }
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
    printf("Client connected.\n\n");

    parse_message(buffer, task_opt_ptr, task_pipes_ptr, task_parsed);

    /* printf("Task opt: %d\nTask num pipes: %d\n", task_opt, task_pipes); */
    /* int i=0; */
    /* while(task_parsed[i]!=NULL) */
    /*   printf("%s\n", task_parsed[i++]); */

    switch (task_opt) {
    case 900:
      exec_task(task_parsed, task_pipes, channel_output);
      timeout = 0;
      childs_done = 0;
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
