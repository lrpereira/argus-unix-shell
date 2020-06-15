#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

/**
   Macros definitions.
*/
#define UNUSED(x) (void)(x)
#define BUFFERSIZE 1024
#define TRUE 111
#define FALSE 222
#define STDIN 0
#define STDOUT 1

/**
   Server funs definitions.
*/
void parse_message(char *buffer, int *task_opt, int *task_pipes_ptr, char *parsed[10]);
void exec_task(char *parsed[10], int task_pipes, char* channel_output);

/**
   Client funs definitions.
*/
void set_inactivity_timelimit(char* buffer);
void set_execution_timelimit(char *buffer);
void history();
void list_running_execs();
void end_task_n();
void run_task(char *buffer);
void cli_mode();

/**
   Auxiliar library funs definition (lib.c).
*/
void help_daemon(int argc);
void help_client();

void create_channel(char *channel);
void send_message(char* channel, int token, char *message);
void receive_message(char* channel, char* buffer);

void clean_command(char* command);
char **parse_command(char *command);
void clean_quotes(char *command);
int get_buffer_size(char *buffer);
void set_string_end(char* string);











