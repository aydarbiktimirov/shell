#pragma once
#include "../modules/memory_control.h"
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "execute.h"
#include "variables.h"
#include "cd.h"
#include "readline.h"
#include "history.h"
#include "../modules/string.h"
#include "../modules/calculator.h"
#include "../modules/settings.h"

void start_shell(char **);
void prompt_shell(struct string);
int read_command(struct string *, char *, char *, size_t *);
