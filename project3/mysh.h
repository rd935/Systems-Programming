#include "arraylist.h"

#ifndef MYSH_DEBUG
#define MYSH_DEBUG 0
#endif

typedef enum _mode_type {
	BATCH,
	INTERACTIVE,
	INVALID
} mode;

typedef struct _job_info {
	char* name;
	char* path_in;
	char* path_out;
	arrlist* args;
	int pipe_in;
	int pipe_out;
} job;

// global variables
mode modeVar = INVALID;
int fd; 
int exit_shell = 0;	
int prev_return = -1;
int pid;

int init(int, char**);
int is_dir (char*);

char* read_line(char*);

int check_cond(arrlist*);
int run_job(arrlist*, int, int);
int parse_args (arrlist*, job*);

int set_in(job*);
int set_out(job*);

void print_job(job*);
char* null_wrapper(char*);
void clear_job(job*);