#include "settings.h"

static char settings_file[FILENAME_MAX] = DEFAULT_CONFIG;
static char ***variables = NULL;

void clear_settings()
{
	size_t i;
	if (variables != NULL)
	{
		for (i = 0; variables[i] != NULL; ++i)
		{
			free(variables[i][0]);
			free(variables[i][1]);
			free(variables[i]);
		}
	}
	variables = (char ***)realloc(variables, sizeof(char **));
	variables[0] = NULL;
}

void free_settings()
{
	clear_settings();
	free(variables);
}

void set_settings_file(const char *file)
{
	int f, index = 0, count = 0;
	char c, quote;
	struct string value;
	clear_settings();
	strcpy(settings_file, file);
	make_string(&value);
	if ((f = open(settings_file, O_RDONLY)) == -1)
	{
		perror("set_settings_file.open");
		return;
	}
	while (read(f, &c, 1) == 1)
	{
		switch (c)
		{
			case '#': /* comment line */
				while (read(f, &c, 1) == 1 && c != '\n'); /* skip it */
				break;
			case '\'':
			case '"':
				quote = c;
				while (read(f, &c, 1) == 1 && c != quote)
				{
					append_string_char(&value, c);
				}
				break;
			case '\\':
				if (read(f, &c, 1) == 1)
				{
					append_string_char(&value, c);
				}
				break;
			case ' ':
			case '\t':
			case '\n':
				/* printf("%s\n", value.str); */
				if (strlen(value.str) != 0)
				{
					if (index == 0)
					{
						variables = (char ***)realloc(variables, (count + 2) * sizeof(char **));
						if (variables == NULL)
						{
							perror("set_settings_file.realloc");
							exit(EXIT_FAILURE);
						}
						variables[count + 1] = NULL;
						variables[count] = (char **)realloc(variables[count], 2 * sizeof(char *));
						if (variables[count] == NULL)
						{
							perror("set_settings_file.realloc");
							exit(EXIT_FAILURE);
						}
						variables[count][0] = variables[count][1] = NULL;
						variables[count][0] = (char *)realloc(variables[count][0], (strlen(value.str) + 1) * sizeof(char));
						if (variables[count][0] == NULL)
						{
							perror("set_settings_file.realloc");
							exit(EXIT_FAILURE);
						}
						strcpy(variables[count][0], value.str);
						/* printf("%s = ", variables[count][0]); */
					}
					else
					{
						variables[count][1] = (char *)realloc(variables[count][1], (strlen(value.str) + 1) * sizeof(char));
						if (variables[count][1] == NULL)
						{
							perror("set_settings_file.realloc");
							exit(EXIT_FAILURE);
						}
						strcpy(variables[count][1], value.str);
						/* printf("%s\n", variables[count][1]); */
						++count;
					}
					index = 1 - index;
					clear_string(&value);
				}
				break;
			default:
				append_string_char(&value, c);
		}
	}
	free_string(&value);
}

void get_settings_file(char *file)
{
	strcpy(file, settings_file);
}

int read_settings_string(struct string *dest, char *variable)
{
	size_t i;
	if (variables == NULL) /* clear settings file */
	{
		return 0;
	}
	for (i = 0; variables[i] != NULL; ++i)
	{
		if (strcmp(variables[i][0], variable) == 0)
		{
			string(dest, variables[i][1]);
			return 0;
		}
	}
	return -1;
}

int read_settings_int(int *dest, char *variable)
{
	int result, value = 0;
	char c;
	struct string temp;
	make_string(&temp);
	if ((result = read_settings_string(&temp, variable)) != 0)
	{
		free_string(&temp);
		return result;
	}
	if (strlen(temp.str) > 0 && sscanf(temp.str, "%d%c", &value, &c) != 1)
	{
		free_string(&temp);
		return -4;
	}
	free_string(&temp);
	*dest = value;
	return 0;
}

int read_settings_bool(char *dest, char *variable)
{
	int result;
	struct string temp;
	make_string(&temp);
	if ((result = read_settings_string(&temp, variable)) != 0)
	{
		free_string(&temp);
		return result;
	}
	*dest = strlen(temp.str) == 0 || strcmp(temp.str, "0") == 0 || strcasecmp(temp.str, "false") == 0 || strcasecmp(temp.str, "no") == 0 ? 0 : 1;
		/* `0`, `false`, `no` and empty string  - false
		 * other variants - true */
	free_string(&temp);
	return 0;
}
