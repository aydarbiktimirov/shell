#pragma once
#include "../modules/memory_control.h"
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void add_to_history(char *);
ssize_t find_history_prev(char *, ssize_t, char **);
ssize_t find_history_next(char *, ssize_t, char **);
void free_history();
