#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "arraylist.h"
#include "tokenizer.h"
#include "mysh.h"


int main(int argc, char **argv) {
	if (init(argc, argv) == EXIT_FAILURE) {
		printf("Exit Failure");
		return EXIT_FAILURE;
	}

	// main input loop
	do {
		if (modeVar == INTERACTIVE) {
			printf("mysh> ");
			fflush(stdout);
		}
		char* input = read_line(input);
		if (input == NULL) {
			if (MYSH_DEBUG && exit_shell != 1) {
				fprintf(stderr, "ERROR: read_line returns null\n");
			}
			break;
		}

		if (MYSH_DEBUG) {
			printf(";%s;\n", input);
		}

		if (strlen(input) != 0) {
			arrlist* arraylist = arr_create(1);
			int tokenizer_rv = tokenizer(arraylist, input) != 0;
			free(input);

			if (MYSH_DEBUG) {
				arr_print(arraylist);
			}
			
			if (tokenizer_rv != 0) {
				if(arraylist->len > 0) {
					if (check_cond(arraylist) == EXIT_SUCCESS) {
						int pipes = arr_contains(arraylist, "|");
						if (pipes == 0) {
							run_job(arraylist, -1, -1);
						} else if (pipes == 1) {
							arrlist* left = arraylist;
							arrlist* right = arr_create(1);
							int found_pipe = 0;

							for (int i = 0; i < arraylist->len; i++) {
								char* curr = arr_get(arraylist, i);
								if (strcmp(curr, "|") == 0) {
									found_pipe = 1;

									if (i == 0 || strcmp(arr_get(arraylist, i - 1), "<") == 0 || strcmp(arr_get(arraylist, i - 1), ">") == 0) {
										fprintf(stderr, "mysh: syntax error\n");
										prev_return = EXIT_FAILURE;
										break;
									} else if (i == arraylist->len - 1 || strcmp(arr_get(arraylist, i + 1), "then") == 0 || strcmp(arr_get(arraylist, i + 1), "else") == 0) {
										fprintf(stderr, "mysh: syntax error\n");
										prev_return = EXIT_FAILURE;
										break;
									}

									arr_remove(left, i);
									i--;
								} else if (found_pipe == 1) {
									arr_push(right, arr_get(arraylist, i));
									arr_remove(left, i);
									i--;
								}								
							}

							if (MYSH_DEBUG) {
								arr_print(left);
								arr_print(right);
							}

							int pipe_fd[2] = {0, 0};
							if (pipe(pipe_fd) == 0) {
								run_job(left, -1, pipe_fd[1]);
								run_job(right, pipe_fd[0], -1);
							} else {
								fprintf(stderr, "Error occurred when creating pipe\n");
							}

							arr_destroy(right);
						} else if (pipes > 1) {
							printf("mysh: mysh does not accept more than one pipe\n");
						}
					} else {
						prev_return = EXIT_FAILURE;
					}
				} else {
					fprintf(stderr, "ERROR: <= 0 tokens returned\n");
					arr_destroy(arraylist);
					break;
				}
			} else {
				prev_return = EXIT_FAILURE;
			}
			arr_destroy(arraylist);
		}
		if (exit_shell) {
			break;
		}
	} while (1);
	
	if (modeVar == BATCH) {
		if (close(fd) == -1) {
			fprintf(stderr, "ERROR: Cannot close %s: %s\n", argv[1], strerror(errno));
		}
	}
	if (exit_shell) {
		printf("Exit Success");
		return EXIT_SUCCESS;
	} else {
		printf("Exit Failure");
		return EXIT_FAILURE;
	}
}

int run_job(arrlist* tokens, int pipe_infd, int pipe_outfd) {
	job task;

	task.name = NULL;
	task.args = tokens;
	task.path_in = NULL;
	task.path_out = NULL;
	task.pipe_in = -1;
	task.pipe_out = -1;

	if (pipe_infd != -1) {
		task.pipe_in = pipe_infd;
	}
	if (pipe_outfd != -1) {
		task.pipe_out = pipe_outfd;
	}

	int parse_args_rv = parse_args(tokens, &task);

	if (MYSH_DEBUG) {
		print_job(&task);
	}

	if (parse_args_rv == EXIT_SUCCESS) {
		if (task.name == NULL) {
			pid = fork();
			if (pid == 0) {
				set_in(&task);
				set_out(&task);
				exit(EXIT_SUCCESS);
			} else {
				wait(&prev_return);
				clear_job(&task);	
				return EXIT_SUCCESS;
			}
		} else if (strchr(task.name, '/') != NULL) {
			int file_exists = 0, perms = 0;
			if (access(task.name, F_OK) == 0) {
				file_exists = 1;
				if(access(task.name, X_OK) == 0) {
					pid = fork();
					if (pid == 0) {
						set_in(&task);
						set_out(&task);

						arr_push(task.args, "");
						void* temp = task.args->h[task.args->len - 1];
						task.args->h[task.args->len - 1] = '\0';
						

						if (MYSH_DEBUG) {
							arr_print(task.args);
						}

						int exec_rv = execv(task.name, task.args->h);
						if (exec_rv != 0) {
							free(temp);
							printf("ERROR: %s\n", strerror(errno));
							exit(errno);
						}
						
					} else {
						wait(&prev_return);
					}

					clear_job(&task);	
					return EXIT_FAILURE;
				}
			}

			if (!file_exists || !perms) {
				fprintf(stderr, "mysh: %s: %s\n", task.name, strerror(errno));
			}
			clear_job(&task);
			prev_return = 0;
			return EXIT_FAILURE;
		} else if (strcmp(task.name, "exit") == 0) {
			if (task.path_out != NULL) {
				if (fork() == 0) {
					set_out(&task);
					exit(EXIT_SUCCESS);
				} else {
					wait(&prev_return);
				}
			}
			clear_job(&task);
			printf("mysh: exiting\n");
			exit_shell = 1;
			return EXIT_SUCCESS;
		} else if (strcmp(task.name, "pwd") == 0) {
			int id = 1;
			id = fork();
			if (id == 0) {
				set_out(&task);

				char* wd = getcwd(NULL, 256);
				if (wd != NULL) {
					printf("%s\n", wd);
					free(wd);
					exit(EXIT_SUCCESS);
				} else {
					fprintf(stderr, "ERROR: getcwd: %s\n", strerror(errno));
					exit(errno);
				}
			} else {
				wait(&prev_return);
				clear_job(&task);
				return prev_return;
			}
		} else if (strcmp(task.name, "cd") == 0) {
			if (task.args->len == 2) {
				if (chdir(arr_get(task.args, 1)) != 0) {
					fprintf(stderr, "mysh: %s: %s\n", task.name, strerror(errno));
					clear_job(&task);
					return EXIT_FAILURE;
				}
				clear_job(&task);
				return EXIT_SUCCESS;
			} else {
				fprintf(stderr, "mysh: cd: expected 1 argument, got %d\n", task.args->len - 1);
				clear_job(&task);
				return EXIT_FAILURE;
			}
		} else {
			int which = 0;
			if (strcmp(task.name, "which") == 0) {
				which = 1;
				if (task.args->len != 2 || strcmp(arr_get(task.args, 1), "cd") == 0 || strcmp(arr_get(task.args, 1), "pwd") == 0 || strcmp(arr_get(task.args, 1), "which") == 0) {
					clear_job(&task);
					return EXIT_FAILURE;
				} else {
					free(task.name);
					task.name = malloc(strlen(arr_get(task.args, 1)) + 1);
					strcpy(task.name, arr_get(task.args, 1));
				}
			}
			const char* dirs[3];
			dirs[0] = "/usr/local/bin/";
			dirs[1] = "/usr/bin/";
			dirs[2] = "/bin/";

			int file_exists = 0, perms = 0;

			for (int i = 0; i < 3; i++) {
				char* path = malloc(strlen(task.name) + 20);
				strcpy(path, dirs[i]);
				strcat(path, task.name);

				if (MYSH_DEBUG) {
					printf(";%s;\n", path);
				}

				if (access(path, F_OK) == 0) {
					file_exists = 1;
					if(access(path, X_OK) == 0) {
						pid = fork();
						if (pid == 0) {
							set_in(&task);
							set_out(&task);

							if (which) {
								printf("%s\n", path);
								free(path);
								exit(EXIT_SUCCESS);
							} else {
								arr_push(task.args, "");
								void* temp = task.args->h[task.args->len - 1];
								task.args->h[task.args->len - 1] = '\0';
								
								if (MYSH_DEBUG) {
									arr_print(task.args);
								}

								int exec_rv = execv(path, task.args->h);
								if (exec_rv != 0) {
									printf("ERROR: %s\n", strerror(errno));
									free(temp);
									exit(errno);
								}
							}
						} else {
							wait(&prev_return);
						}

						free(path);
						clear_job(&task);	
						return prev_return;
					}
				}
				free(path);
			}

			if (!file_exists && which == 0) {
				if (!perms) {
					printf("mysh: %s: No such file or directory\n", task.name);
				} else {
				printf("mysh: %s: Permission denied\n", task.name);
				}
			}
			clear_job(&task);
			return EXIT_FAILURE;
		}			
	} else {
		return EXIT_FAILURE;
	}
}

char* read_line (char* buf) {
	buf = malloc(4);
	int len = 0, cap = 4;
	char* i = buf;

	while (len == 0 || *(i - 1) != '\n') {
		if (len > cap - 1) {
			cap = cap * 2;
			buf = realloc(buf, cap);
			i = buf + len;
		}
		int bytes_read = read(fd, i, 1);
		
		if (bytes_read == -1) {
			fprintf(stderr, "ERROR: Cannot read from file: %s\n", strerror(errno));
			free(buf);
			return NULL;
		} else if (bytes_read == 0) {
			exit_shell = 1;
			if (len == 0) {
				free(buf);
				return NULL;
			}
			*i = '\0';
			return buf;
		} else if (*i == '\n') {
			if (len == 0) {
				i--;
			} else {
				*i = '\0';
				return buf;
			}
		} else {
			len++;
		}
		i++;
	}

	fprintf(stderr, "ERROR: This loop isn't supposed to terminate\n");
	free(buf);
	return NULL;
}

// configures shell before it can take user input
// sets mode and validates arguments
int init(int argc, char** argv) {
	if (argc == 1) {
		printf("Starting mysh in interactive mode\n");
		fd = STDIN_FILENO;
		modeVar = INTERACTIVE;
		return EXIT_SUCCESS;
	} else if (argc == 2) {
		if (access(argv[1], R_OK) == 0) {
			if (is_dir(argv[1]) == 0) {
				fd = open(argv[1], O_RDONLY);
				if (fd > 0) {
					modeVar = BATCH;
					return EXIT_SUCCESS;
				}
			} 
		}
	} else {
		errno = E2BIG;
	}

	fprintf(stderr, "mysh: ");
	if (argc == 2) {
		fprintf(stderr, "%s: ", argv[1]);
	}
	fprintf(stderr, "%s\n", strerror(errno));
	return EXIT_FAILURE;
}

// return 0 if arg is not a dir
// return 1 if arg is a dir
int is_dir (char* arg) {
	struct stat file_buf;
	if (stat(arg, &file_buf) == 0) {
		if (S_ISDIR(file_buf.st_mode) == 0) {
			return 0;
		} else {
			errno = EISDIR;
			return 1;
		}
	} else {
		return 1;
	}
}

int check_cond(arrlist* arraylist) {
	if (strcmp(arr_get(arraylist, 0), "then") == 0) {
		if (arraylist->len == 1) {
			fprintf(stderr, "mysh: syntax error: list a command after then\n");
			return EXIT_FAILURE;				
		}

		if (prev_return == EXIT_SUCCESS) {
			arr_remove(arraylist, 0);
			return EXIT_SUCCESS;
		} else if (prev_return == -1) {
			fprintf(stderr, "mysh: cannot run conditional job when there is no previous state\n");
		}
		return EXIT_FAILURE;
	} else if (strcmp(arr_get(arraylist, 0), "else") == 0) {
		if (arraylist->len == 1) {
			fprintf(stderr, "mysh: syntax error: list a command after else\n");
			return EXIT_FAILURE;
		}

		if (prev_return != EXIT_SUCCESS && prev_return != -1) {
			arr_remove(arraylist, 0);
			return EXIT_SUCCESS;
		} else if (prev_return == -1) {
			fprintf(stderr, "mysh: cannot run conditional job when there is no previous state\n");
		}
		return EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}
}

int parse_args (arrlist* tokens, job* task) {
	if (tokens->len < 1) {
		fprintf(stderr, "mysh: syntax error\n");
		return EXIT_FAILURE;
	}

	// process redirects
	for (int i = 0; i < tokens->len; i++) {
		if (strcmp(arr_get(tokens, i), ">") == 0) {
			if (i < tokens->len - 1) {
				if (task->path_out != NULL) {
					free(task->path_out);
					task ->path_out = NULL;
				}
				task->path_out = malloc(strlen(arr_get(tokens, i + 1)) + 1);
				strcpy(task->path_out, arr_get(tokens, i + 1));
				arr_remove(tokens, i);
				arr_remove(tokens, i);
				i -= 1;
			} else {
				fprintf(stderr, "mysh: syntax error\n");
				return EXIT_FAILURE;
			}
		} else if (strcmp(arr_get(tokens, i), "<") == 0) {
			if (i < tokens->len - 1) {
				if (task->path_in != NULL) {
					free(task->path_in);
					task ->path_in = NULL;
				}
				task->path_in = malloc(strlen(arr_get(tokens, i + 1)) + 1);
				strcpy(task->path_in, arr_get(tokens, i + 1));
				printf("%s\n", task->path_in);
				arr_remove(tokens, i);
				arr_remove(tokens, i);
				i -= 1;
			} else {
				fprintf(stderr, "mysh: syntax error\n");
				return EXIT_FAILURE;
			}
		}
	}

	// find job name
	for (int i = 0; i < tokens->len; i++) {
		task->name = malloc(strlen(arr_get(tokens, i)) + 1);
		strcpy(task->name, arr_get(tokens, i));
		break;
	}

	return EXIT_SUCCESS;
}

void print_job(job* task) {
	fprintf(stderr, "=== JOB ===\n");
	fprintf(stderr, "NAME: %s\n", null_wrapper(task->name));
	fprintf(stderr, "STDIN: %s\n", null_wrapper(task->path_in));
	fprintf(stderr, "STDOUT: %s\n", null_wrapper(task->path_out));
	fprintf(stderr, "PIPE_STD_IN: %d\n", task->pipe_in);
	fprintf(stderr, "PIPE_STD_OUT: %d\n", task->pipe_out);
	fprintf(stderr, "ARGS:\n");
	arr_print(task->args);
	fprintf(stderr, "===========\n");
}

void clear_job(job* task) {
	if (task == NULL) {
		return;
	}
	if (task->name != NULL){
		free(task->name);
	}
	if (task->path_in != NULL) {
		free(task->path_in);
	}
	if(task->path_out != NULL) {
		free(task->path_out);
	}
}

int set_out (job* task) {
	if (task->path_out != NULL) {
		if (access(task->path_out, W_OK) == 0) {
			int std_out = open(task->path_out, O_WRONLY | O_TRUNC);
			dup2(std_out, STDOUT_FILENO);
		} else {
			int std_out = open(task->path_out, O_CREAT | O_WRONLY);
			chmod(task->path_out, S_IRUSR|S_IWUSR|S_IRGRP);
			dup2(std_out, STDOUT_FILENO);
		}
	} else if (task->pipe_out != -1) {
		dup2(task->pipe_out, STDOUT_FILENO);
	}
	return EXIT_SUCCESS;
}

int set_in (job* task) {
	if (task->path_in != NULL) {
		if (access(task->path_in, R_OK) == 0) {
			int std_in = open(task->path_in, O_RDONLY);
			dup2(std_in, STDIN_FILENO);
		} else {
			fprintf(stderr, "mysh: %s: %s\n", task->path_in, strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else if (task->pipe_in != -1) {
		dup2(task->pipe_in, STDIN_FILENO);
	}
	return EXIT_SUCCESS;
}

char* null_wrapper(char* field) {
	if (field != NULL) {
		return field;
	} else {
		return "NULL";
	}
}