#include "shell.h"

static char char_in_string(char c, char *s)
{
	char tmp[2] = {0, 0};
	tmp[0] = c;
	/*printf("-%d-\n", strspn(tmp, s));*/
	return (strspn(tmp, s) > 0 ? 1 : 0);
}

/*static void seek_string()
{
	char c;
	return;
	while ((c = getchar()) != EOF && c != '\n');
}*/

static void replace_char_codes(struct string *str) /* replaces all \n \t \0XXX \xXX chars */
{
	size_t i;
	struct string tmp, res;
	/*if (strlen(str->str) == 0)
	{
		return;
	}*/
	make_string(&tmp);
	make_string(&res);
	for (i = 0; i + 1 < strlen(str->str); ++i)
	{
		if (str->str[i] == '\\')
		{
			clear_string(&tmp);
			clear_string(&res);
			switch (str->str[i + 1])
			{
				case 'n':
					append_string_char(&tmp, '\n');
					string(&res, "\\n");
					replace_string(str, str, &res, &tmp);
					break;
				case 't':
					append_string_char(&tmp, '\t');
					string(&res, "\\t");
					replace_string(str, str, &res, &tmp);
					break;
				case '0':
					if (i + 2 < strlen(str->str) && char_in_string(str->str[i + 2], "01234567"))
					{
						if (i + 3 < strlen(str->str) && char_in_string(str->str[i + 3], "01234567"))
						{
							if (i + 4 < strlen(str->str) && char_in_string(str->str[i + 4], "01234567")) /* \0XXX char */
							{
								sub_string(&tmp, str, i, 5);
								append_string_char(&res, ((str->str[i + 2] - '0') * 8 + str->str[i + 3] - '0') * 8 + str->str[i + 4] - '0');
							}
							else /* \0XX char */
							{
								sub_string(&tmp, str, i, 4);
								append_string_char(&res, (str->str[i + 2] - '0') * 8 + str->str[i + 3] - '0');
							}
						}
						else /* \0X char */
						{
							sub_string(&tmp, str, i, 3);
							append_string_char(&res, str->str[i + 2] - '0');
						}
						replace_string(str, str, &tmp, &res);
					}
					break;
				case 'x':
					if (i + 2 < strlen(str->str) && char_in_string(toupper(str->str[i + 2]), "0123456789ABCDEF"))
					{
						if (i + 3 < strlen(str->str) && char_in_string(toupper(str->str[i + 2]), "0123456789ABCDEF")) /* \xXX char */
						{
							sub_string(&tmp, str, i, 4);
							append_string_char(&res, (str->str[i + 2] >= '0' && str->str[i + 2] <= '9' ? str->str[i + 2] - '0' : toupper(str->str[i + 2]) - 'A' + 10) * 16 + (str->str[i + 3] >= '0' && str->str[i + 3] <= '9' ? str->str[i + 3] - '0' : toupper(str->str[i + 3]) - 'A' + 10));
						}
						else /* \xX char */
						{
							sub_string(&tmp, str, i, 3);
							append_string_char(&res, str->str[i + 2] >= '0' && str->str[i + 2] <= '9' ? str->str[i + 2] - '0' : toupper(str->str[i + 2]) - 'A' + 10);
						}
						replace_string(str, str, &tmp, &res);
					}
					break;
			}
		}
	}
	free_string(&res);
	free_string(&tmp);
}

static enum {STDCMD_NONE, STDCMD_EXIT, STDCMD_CD} check_standard_command(struct string *cmd)
{
	/* size_t i, j; */
	int result = STDCMD_NONE;
	/* struct string tmp;
	make_string(&tmp);
	for (i = 0; i < strlen(cmd->str) && char_in_string(cmd->str[i], " \t\n"); ++i);
	for (j = 0; i + j < strlen(cmd->str) - 1 && !char_in_string(cmd->str[i + j], " \t\n"); ++j);
	sub_string(&tmp, cmd, i, j);
	if (strcmp(tmp.str, "exit") == 0)
	{
		result = STDCMD_EXIT;
	}
	if (strcmp(tmp.str, "cd") == 0)
	{
		result = STDCMD_CD;
	}
	free_string(&tmp);*/
	char **cmd_parts = split_cmd(cmd->str);
	size_t i;
	if (cmd_parts[0] != NULL)
	{
		if (strcmp(cmd_parts[0], "exit") == 0)
		{
			result = STDCMD_EXIT;
		}
		if (strcmp(cmd_parts[0], "cd") == 0)
		{
			result = STDCMD_CD;
		}
	}
	for (i = 0; cmd_parts[i] != NULL; ++i)
	{
		free(cmd_parts[i]);
	}
	free(cmd_parts);
	return result;
}

static int run(struct string *_cmd, struct string *result, char **envp)
{
	struct string cmd;
	make_string(&cmd);
	copy_string(&cmd, _cmd);
	/* copy_string(result, &cmd); *//* run a command and write its output into result */
	execute_into_string(&cmd, envp, STDIN_FILENO, result);
	free_string(&cmd);
	return 0;
}

static char prepare_command(struct string *_cmd, char **envp, size_t start, char _run)
{
	size_t i, j, sum;
	int result;
	struct string sub_cmd, cmd/*, var*/;
	char calc_res[255];
	make_string(&cmd);
	make_string(&sub_cmd);
	/* make_string(&var); */
	copy_string(&cmd, _cmd);
	for (i = start; i < strlen(cmd.str); ++i)
	{
		/* skip 'string' */
		if (cmd.str[i] == '\'')
		{
			for (++i; i < strlen(cmd.str) && cmd.str[i] != '\''; ++i);
		}
		if (i > 0 && cmd.str[i - 1] == '$' && cmd.str[i] == '(')
		{
			if (i + 1 < strlen(cmd.str) && cmd.str[i + 1] == '(') /* $(( expression )) */
			{
				sum = 1; /* number of opened brackets */
				for (j = i + 2; j < strlen(cmd.str) && sum > 0; ++j)
				{
					if (cmd.str[j] == '(')
					{
						++sum;
					}
					if (cmd.str[j] == ')')
					{
						--sum;
						if (sum == 0)
						{
							if (j + 1 == strlen(cmd.str) || cmd.str[j + 1] != ')') /* $(( ... ) */
							{
								return -1;
							}
							else /* i - start, j - end */
							{
								sub_string(&sub_cmd, &cmd, i + 2, j - i - 2);
								prepare_command(&sub_cmd, envp, 0, 0);
								/* printf("$((%s)) at [%d, %d]\n", sub_cmd.str, i, j); */
								/*printf("%s -> ", sub_cmd.str);*/
								if (calculate(sub_cmd.str, &result) != 0)
								{
									free_string(&sub_cmd);
									free_string(&cmd);
									return -1;
								}
								/*printf("%d\n", result);*/
								sprintf(calc_res, "%d", result);
								string(&sub_cmd, calc_res);
								replace_string_part(&cmd, &cmd, &sub_cmd, i - 1, j + 2);
								i += strlen(sub_cmd.str) - 1;
								/*printf("start from %d\n", i);*/
							}
						}
					}
				}
			}
			else /* $( sub_command ) */
			{
				sum = 1;
				for (j = i + 1; j < strlen(cmd.str) && sum > 0; ++j)
				{
					if (cmd.str[j] == '(')
					{
						++sum;
					}
					if (cmd.str[j] == ')')
					{
						--sum;
						if (sum == 0)
						{
							sub_string(&sub_cmd, &cmd, i + 1, j - i - 1);
							prepare_command(&sub_cmd, envp, 0, 1);
							/* printf("$(%s) at [%d, %d]\n", sub_cmd.str, i, j); */
							/*insert_string_first_char(&sub_cmd, '"');
							append_string_char(&sub_cmd, '"');*/
							replace_string_part(&cmd, &cmd, &sub_cmd, i - 1, j + 1);
							i += strlen(sub_cmd.str) - 1;
						}
					}
				}
			}
		}
		if (i > 0 && cmd.str[i - 1] == '$' && cmd.str[i] == '{') /* ${variable} */
		{
			for (j = i + 1; j < strlen(cmd.str); ++j)
			{
				if (cmd.str[j] == '}')
				{
					sub_string(&sub_cmd, &cmd, i + 1, j - i - 1);
					/* printf("var %s\n", var.str); */
					get_variable(&sub_cmd, &sub_cmd); /* ? */
					/* prepare_command(&var, 0, 1); */
					/* string(&var, "test"); */
					insert_string_first_char(&sub_cmd, '"');
					append_string_char(&sub_cmd, '"');
					replace_string_part(&cmd, &cmd, &sub_cmd, i - 1, j + 1);
					i += strlen(sub_cmd.str) - 1;
					break;
				}
			}
		}
	}
	if (_run == 1)
	{
		run(&cmd, &cmd, envp);
	}
	copy_string(_cmd, &cmd);
	/* free_string(&var); */
	free_string(&sub_cmd);
	free_string(&cmd);
	return 0;
}

static enum {RESULT_SUCCESS = 0, RESULT_FAIL, RESULT_EXIT} shell_execute(struct string cmd, char **envp)
{
	size_t i;
	char **cmd_parts;
	if (prepare_command(&cmd, envp, 0, 0) != 0) /* replace all $() and $(()). ${} ? */
	{
		return RESULT_FAIL;
	}
	/* replace all {X..Y} {X,Y} */
	/* replace ~ */
	/* - replace all \n \t \0XXX \xXX chars */
	switch (check_standard_command(&cmd))
	{
		case STDCMD_NONE:
			/*printf("returned %d\n", execute(&cmd, envp, STDIN_FILENO, STDOUT_FILENO));*/
			execute(&cmd, envp, STDIN_FILENO, STDOUT_FILENO);
			/*printf("%s", cmd.str);*/
			break;
		case STDCMD_EXIT:
			printf("Bye.\n");
			return RESULT_EXIT;
			break;
		case STDCMD_CD:
			cmd_parts = split_cmd(cmd.str);
			cd(cmd_parts[1]);
			for (i = 0; cmd_parts[i] != NULL; ++i)
			{
				free(cmd_parts[i]);
			}
			free(cmd_parts);
			break;
	}
	return RESULT_SUCCESS;
}

static char prepare_ps1(struct string *ps1, char **envp)
{
	if (prepare_command(ps1, envp, 0, 0) != 0) /* replace all $() and $(()). ${} ? */
	{
		return -1;
	}
	/* replace all {X..Y} {X,Y} */
	/* replace ~ */
	replace_char_codes(ps1); /* replace all \n \t \0XXX \xXX chars */
	return 0;
}

void start_shell(/*const char *settings_file, */char **envp)
{
	struct string _ps1, ps1, cmd;
	char *line = NULL;
	int status = 0;
	size_t i;
	make_string(&ps1);
	make_string(&_ps1);
	read_settings_string(&_ps1, /*settings_file, */"PS1");
	/*initscr();
	nonl();
	cbreak();
	keypad(stdscr, 1);*/
	while (1)
	{
		make_string(&cmd);
		copy_string(&ps1, &_ps1);
		prepare_ps1(&ps1, envp);
		prompt_shell(ps1);
		clear_string(&cmd);
		read_line(&line);
		i = 0;
		if (read_command(&cmd, "\n", line, &i) != 0)
		{
			fprintf(stderr, "Incorrect input\n");
		}
		else
		{
			set_return_status(status);
			switch (shell_execute(cmd, envp))
			{
				case RESULT_FAIL:
					fprintf(stderr, "Incorrect command\n");
					break;
				case RESULT_EXIT:
					/* free(cmd.str); */
					free_string(&_ps1);
					free_string(&ps1);
					/*endwin();*/
					free(line);
					return;
				default:
					break;
			}
			status = get_return_status();
			/* free(cmd.str); */
		}
	}
	/*endwin();*/
}

void prompt_shell(struct string ps1)
{
	printf("%s", ps1.str);
}

int read_command(struct string *cmd, char *endl, char *line, size_t *i)
{
	char c;
	while ((c = /*getchar()*/line[(*i)++]) != /*EOF*/0 && !char_in_string(c, endl))
	{
		if (c != '\\')
		{
			append_string_char(cmd, c);
		}
		switch (c)
		{
			case '\\':
				if ((c = /*getchar()*/line[(*i)++]) == /*EOF*/0)
				{
					/*seek_string();*/
					return -1;
				}
				append_string_char(cmd, '\'');
				append_string_char(cmd, c);
				append_string_char(cmd, '\'');
				if (c == '\n')
				{
					printf("> ");
				}
				break;
			/*case '\n':
				printf("> ");
				break;*/
			case '\'':
				while ((c = /*getchar(*/line[(*i)++]) != /*EOF*/0 && c != '\'')
				{
					append_string_char(cmd, c);
					if (c == '\n')
					{
						printf("> ");
					}
				}
				append_string_char(cmd, c);
				break;
			case '(':
				if (read_command(cmd, ")", line, i) == -1)
				{
					/*seek_string();*/
					return -1;
				}
				break;
			case '{':
				if (c == '\n')
				{
					printf("> ");
				}
				if (read_command(cmd, "}", line, i) == -1)
				{
					/*seek_string();*/
					return -1;
				}
				break;
			case '"':
				if (read_command(cmd, "\"", line, i) == -1)
				{
					/*seek_string();*/
					return -1;
				}
				break;
			default:
				if (char_in_string(c, ")}") > 0)
				{
					/*seek_string();*/
					return -1;
				}
		}
	}
	if (c != /*EOF*/0)
	{
		append_string_char(cmd, c);
	}
	return 0;
}
