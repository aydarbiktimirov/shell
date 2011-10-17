#pragma once
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <stdio.h>
#include "../modules/memory_control.h"
#include "history.h"
#define EOLN 13
#define BACKSPACE 127
#define ENDL "\r\n"
#define CONTROL(x) ((x) == 'C' ? 3 : 'D' ? 4 : -1)
#define ARROW_CHAR(x, f) f = (x == 27 ? 1 : f); x = (x == 27 ? ((x = getchar()) ==  91 ? getchar() : x) : x)
#define DELETE_CHAR(x, f) f = (x == 126 ? 1 : f); x = (x == 126 ? getchar() : x)
#define ARROW_UP 65
#define ARROW_DOWN 66
#define ARROW_RIGHT 67
#define ARROW_LEFT 68
#define DELETE 351
#define MOVE_LEFT() printf("\x1B\x5B\x44")
#define MOVE_RIGHT() printf("\x1B\x5B\x43")

void set_raw_terminal_mode();
void set_default_terminal_mode();
size_t read_line(char **);
