#include "history.h"

static char **history = NULL;
static ssize_t history_size = 0;

void add_to_history(char *cmd)
{
	if (strlen(cmd) == 0)
	{
		return;
	}
	++history_size;
	if ((history = (char **)realloc(history, sizeof(char *) * history_size)) == NULL)
	{
		fprintf(stderr, "add_to_history.realloc");
		exit(EXIT_FAILURE);
	}
	if ((history[history_size - 1] = (char *)malloc(sizeof(char) * (strlen(cmd) + 1))) == NULL)
	{
		fprintf(stderr, "add_to_history.malloc");
		exit(EXIT_FAILURE);
	}
	strcpy(history[history_size - 1], cmd);
	
}

ssize_t find_history_prev(char *pattern, ssize_t offset, char **dest)
{
	ssize_t i;
	if (offset == -2)
	{
		offset = 0;
	}
	i = (offset == 0 ? history_size : offset - 1);
	for (; i > 0; --i)
	{
		if (strstr(history[i - 1], pattern) == history[i - 1])
		{
			*dest = (char *)realloc(*dest, sizeof(char) * (strlen(history[i - 1]) + 1));
			strcpy(*dest, history[i - 1]);
			return i;
		}
	}
	*dest = (char *)realloc(*dest, sizeof(char));
	(*dest)[0] = 0;
	return -1;
}

ssize_t find_history_next(char *pattern, ssize_t offset, char **dest)
{
	ssize_t i;
	if (offset <= 0)
	{
		*dest = (char *)realloc(*dest, sizeof(char));
		(*dest)[0] = 0;
		switch (offset)
		{
			case -1:
				offset = 1;
				break;
			case -2:
				return -2;
		}
	}
	for (i = offset; i < history_size; ++i)
	{
		if (strstr(history[i], pattern) == history[i])
		{
			*dest = (char *)realloc(*dest, sizeof(char) * (strlen(history[i]) + 1));
			strcpy(*dest, history[i]);
			return i + 1;
		}
	}
	*dest = (char *)realloc(*dest, sizeof(char));
	(*dest)[0] = 0;
	return -2;
}

void free_history()
{
	size_t i;
	if (history == NULL)
	{
		return;
	}
	for (i = 0; i < (size_t)history_size; ++i)
	{
		free(history[i]);
	}
	free(history);
}
