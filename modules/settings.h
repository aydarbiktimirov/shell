#pragma once
#include "memory_control.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "string.h"
#define DEFAULT_CONFIG ""

void free_settings();
void set_settings_file(const char *);
void get_settings_file(char *);
int read_settings_string(struct string *, char *);
int read_settings_int(int *, char *);
int read_settings_bool(char *, char *);
