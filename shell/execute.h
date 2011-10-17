#pragma once
#include "../modules/memory_control.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include "../modules/string.h"
#include "variables.h"
#define EXIT_EXEC_FAILURE 127

char **split_cmd(char *);
int execute(struct string *, char **, int, int); /* executes command with specified environment, stdin and stdout. return value is return code of command */
int execute_into_string(struct string *, char **, int, struct string *);
int get_return_status();
void set_return_status(int);
