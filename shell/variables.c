#include "variables.h"

static char ***variables = NULL; /* {{"var_name1", "var_value1"}, ..., {"var_nameN", "var_valueN"}, NULL } */

static int get_user_variable(struct string *dest, char *var)
{
	size_t i;
	if (variables == NULL)
	{
		return -1;
	}
	for (i = 0; variables[i] != NULL; ++i)
	{
		if (strcmp(variables[i][0], var) == 0)
		{
			string(dest, variables[i][1]);
			return 0;
		}
	}
	clear_string(dest);
	return -1;
}

void set_variable(struct string *var, struct string *value)
{
	size_t i;
	for (i = 0; variables[i] != NULL; ++i)
	{
		if (strcmp(variables[i][0], var->str) == 0)
		{
			variables[i][1] = (char *)realloc(variables[i][1], (strlen(value->str) + 1) * sizeof(char));
			strcpy(variables[i][1], value->str);
			return;
		}
	}
	variables = (char ***)realloc(variables, (i + 2) * sizeof(char **));
	variables[i] = (char **)realloc(variables[i], 2 * sizeof(char *));
	variables[i][0] = (char *)malloc((strlen(var->str) + 1) * sizeof(char));
	variables[i][1] = (char *)malloc((strlen(value->str) + 1) * sizeof(char));
	variables[i + 1] = NULL;
	strcpy(variables[i][0], var->str);
	strcpy(variables[i][1], value->str);
}

static void get_home(struct string *dest)
{
	struct passwd *result;
	if ((result = getpwuid(getuid())) == NULL)
	{
		perror("get_home.getpwuid");
		return;
	}
	string(dest, result->pw_dir);
}

static void get_string_return_status(struct string *dest)
{
	int status = get_return_status();
	clear_string(dest);
	do
	{
		append_string_char(dest, '0' + (status % 10));
		status /= 10;
	}
	while (status);
	reverse_string(dest);
}

static int get_special_variable(struct string *dest, char *var)
{
	if (strcmp(var, "HOME") == 0)
	{
		get_home(dest);
		return 0;
	}
	if (strcmp(var, "?") == 0)
	{
		get_string_return_status(dest);
		return 0;
	}
	return -1;
}

void get_variable(struct string *dest, struct string *var)
{
	/* string(dest, "test"); */
	/* return ; */
	if (get_special_variable(dest, var->str) != 0)
	{
		if (read_settings_string(dest, var->str) != 0)
		{
			if (get_user_variable(dest, var->str) != 0)
			{
				clear_string(dest);
			}
		}
	}
}
