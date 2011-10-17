#include "readline.h"

static struct termios raw_term, default_term;

void set_raw_terminal_mode()
{
	tcgetattr(0, &default_term);
	cfmakeraw(&raw_term);
	tcsetattr(0, TCSANOW, &raw_term);
}

void set_default_terminal_mode()
{
	tcsetattr(0, TCSANOW, &default_term);
}

static char check_line(char *str)
{
	size_t i;
	char tmp;
	for (i = 0; i < strlen(str); ++i)
	{
		switch (str[i])
		{
			case '\\':
				if (str[++i] == 0)
				{
					return 0;
				}
				break;
			case '"':
			case '\'':
				tmp = str[i];
				for (++i; str[i] != tmp; ++i)
				{
					if (str[i] == 0)
					{
						return 0;
					}
				}
				++i;
		}
	}
	return 1;
}

size_t read_line(char **dest)
{
	size_t pos = 0, len = 0, i, size = 1, min_pos = 0;
	ssize_t history_index;
	int c, escape_flag, history_flag = 0, break_flag = 0;
	char *pattern, *res = NULL;
	*dest = (char *)realloc(*dest, sizeof(char) * size);
	(*dest)[0] = 0;
	set_raw_terminal_mode();
	pattern = (char *)malloc(sizeof(char));
	pattern[0] = 0;
	while ((c = getchar()) != /*EOLN && c !=*/ CONTROL('D'))
	{
		escape_flag = 0;
		ARROW_CHAR(c, escape_flag);
		if (escape_flag)
		{
			switch (c)
			{
				case ARROW_UP:
					if (!history_flag)
					{
						history_index = 0;
						pattern = (char *)realloc(pattern, sizeof(char) * (strlen(*dest) + 1));
						strcpy(pattern, *dest);
					}
					history_flag = 1;
					history_index = find_history_prev(pattern, history_index, &res);
					if (history_index != -1)
					{
						strcpy(*dest, res);
						for (i = pos; i > 0; --i)
						{
							MOVE_LEFT();
						}
						printf("%s", *dest);
						for (i = strlen(*dest); i < len; ++i)
						{
							printf(" ");
						}
						for (i = strlen(*dest); i < len; ++i)
						{
							MOVE_LEFT();
						}
						len = pos = strlen(*dest);
						size = len + 1;
					}
					break;
				case ARROW_DOWN:
					if (!history_flag)
					{
						history_index = 0;
						pattern = (char *)realloc(pattern, sizeof(char) * (strlen(*dest) + 1));
						strcpy(pattern, *dest);
					}
					history_flag = 1;
					history_index = find_history_next(pattern, history_index, &res);
					if (history_index == -2)
					{
						strcpy(res, pattern);
					}
					strcpy(*dest, res);
					for (i = pos; i > 0; --i)
					{
						MOVE_LEFT();
					}
					printf("%s", *dest);
					for (i = strlen(*dest); i < len; ++i)
					{
						printf(" ");
					}
					for (i = strlen(*dest); i < len; ++i)
					{
						MOVE_LEFT();
					}
					len = pos = strlen(*dest);
					size = len + 1;
					break;
				case ARROW_RIGHT:
					if (pos < len)
					{
						++pos;
						MOVE_RIGHT();
					}
					break;
				case ARROW_LEFT:
					if (pos > min_pos)
					{
						--pos;
						MOVE_LEFT();
					}
					break;
				case DELETE:
					history_flag = 0;
					/* copy dest <-> history */
					break;
			}
		}
		else
		{
			history_flag = 0;
			switch (c)
			{
				case EOLN:
					if (check_line(*dest))
					{
						break_flag = 1;
					}
					else
					{
						printf("%s> ", ENDL);
						/*if (len + 1 == size)
						{
							size *= 2;
							(*dest) = (char *)realloc(*dest, sizeof(char) * size);
						}
						(*dest)[len++] = ' ';
						(*dest)[len] = 0;*/
						(*dest)[len - 1] = 0;
						pos = --len;
						min_pos = len;
					}
					break;
				case CONTROL('C'):
					printf("^C%s", ENDL);
					set_default_terminal_mode();
					free(pattern);
					return (*dest)[0] = 0;
				case BACKSPACE:
					if (pos > min_pos)
					{
						for (i = pos - 1; i < len; ++i)
						{
							(*dest)[i] = (*dest)[i + 1];
						}
						(*dest)[len - 1] = 0;
						--pos;
						--len;
						printf("\x8%s ", (*dest) + pos);
						for (i = pos; i <= len; ++i)
						{
							MOVE_LEFT();
						}
					}
					break;
				default:
					if (len + 1 == size)
					{
						size *= 2;
						(*dest) = (char *)realloc(*dest, sizeof(char) * size);
					}
					(*dest)[len + 1] = 0;
					for (i = len; i > pos; --i)
					{
						(*dest)[i] = (*dest)[i - 1];
					}
					(*dest)[pos++] = c;
					++len;
					printf("%s", (*dest) + pos - 1);
					for (i = pos; i < len; ++i)
					{
						MOVE_LEFT();
					}
			}
		}
		if (break_flag)
		{
			break;
		}
	}
	printf(ENDL);
	(*dest)[len] = 0;
	set_default_terminal_mode();
	add_to_history(*dest);
	free(pattern);
	return len;
}
