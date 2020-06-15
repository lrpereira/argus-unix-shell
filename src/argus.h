#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

/** MACROS */
#define UNUSED(x) (void)(x)
#define BUFFERSIZE 1024
#define TRUE 111
#define FALSE 222
#define STDIN 0
#define STDOUT 1

/** LIB */
void help_daemon(int argc);
void create_channel(char *channel);
void clean_command(char* command);
char **parse_command(char *command);
void clean_quotes(char *command);
int get_buffer_size(char *buffer);
void set_string_end(char* string);

/** SERVER */
void exec_task(char *parsed[10], int task_pipes, char* channel_output);
void parse_message(char *buffer, int *task_opt, int *task_pipes_ptr,
                   char *parsed[10]);

/** CLIENT */
void send_message(int fd, int token, char *message);
void receive_message(int fd, char* buffer);

void help_client();
void cli_mode();
void list_running_execs();
void history();

void set_inactivity_timelimit();
void set_execution_timelimit();
void end_task_n();
void run_task();
