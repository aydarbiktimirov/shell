#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "execute.h"
#include "../modules/settings.h"

static int return_status = 0;

int get_return_status()
{
	return return_status;
}

void set_return_status(int new_status)
{
	return_status = new_status;
}

char **split_cmd(char *cmd)
{
	size_t i, count = 0;
	struct string temp;
	char **result = NULL, quote;
	make_string(&temp);
	for (i = 0; i < strlen(cmd); )
	{
		for (; i < strlen(cmd) && (cmd[i] == ' ' || cmd[i] == '\t' || cmd[i] == '\n'); ++i); /* skip spaces */
		clear_string(&temp);
		if (i == strlen(cmd))
		{
			break;
		}
		for (; i < strlen(cmd) && cmd[i] != ' ' && cmd[i] != '\t' && cmd[i] != '\n'; ++i)
		{
			if ((quote = cmd[i]) == '\'' || quote == '"')
			{
				for (++i; i < strlen(cmd) && cmd[i] != quote; ++i)
				{
					append_string_char(&temp, cmd[i]);
				}
			}
			else
			{
				append_string_char(&temp, cmd[i]);
			}
		}
		if ((result = (char **)realloc(result, (count + 1) * sizeof(char *))) == NULL)
		{
			perror("split_cmd.realloc");
			exit(EXIT_FAILURE);
		}
		if ((result[count++] = (char *)malloc((strlen(temp.str) + 1) * sizeof(char))) == NULL)
		{
			perror("split_cmd.malloc");
			exit(EXIT_FAILURE);
		}
		strcpy(result[count - 1], temp.str);
	}
	if ((result = (char **)realloc(result, (count + 1) * sizeof(char *))) == NULL)
	{
		perror("split_cmd.realloc");
		exit(EXIT_FAILURE);
	}
	result[count] = NULL;
	free_string(&temp);
	return result;
}

static char is_executable(char *filename)
{
	struct stat result;
	if (stat(filename, &result) == -1)
	{
		return 0;
	}
	return (result.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH) ? 1 : 0);
}

static void find_executable(char **cmd)
{
	size_t i;
	struct string PATH, current, result;
	make_string(&PATH);
	make_string(&current);
	make_string(&result);
	string(&result, *cmd);
	read_settings_string(&PATH, "PATH");
	append_string_char(&PATH, ':');
	append_string_char(&PATH, ':');
	for (i = 0; i < strlen(PATH.str); ++i)
	{
		if (PATH.str[i] == ':')
		{
			append_string_char(&current, '/');
			append_string(&current, &result);
			if (is_executable(current.str))
			{
				*cmd = (char *)realloc(*cmd, (strlen(current.str) + 1) * sizeof(char));
				strcpy(*cmd, current.str);
				break;
			}
			clear_string(&current);
		}
		else
		{
			append_string_char(&current, PATH.str[i]);
		}
	}
	free_string(&result);
	free_string(&current);
	free_string(&PATH);
}

/*static void prepare_param(char **param)
{
	struct string home, temp;
	if (strlen(*param) == 0)
	{
		return;
	}
	if ((*param)[0] == '~')
	{
		if (strlen(*param) == 1 || (*param)[1] == '/')
		{
			make_string(&home);
			make_string(&temp);
			string(&home, "HOME");
			string(&temp, &((*param)[1]));
			get_variable(&home, &home);
			append_string(&home, &temp);
			*param = (char *)realloc(*param, (strlen(home.str) + 1) * sizeof(char));
			strcpy(*param, home.str);
			free_string(&temp);
			free_string(&home);
		}
	}
}*/

static int execute_cmd(struct string *cmd, char **envp, int input, int output, char wait_child)
{
	pid_t fork_pid;
	size_t i;
	char **cmd_parts = NULL;
	int status = EXIT_SUCCESS;
	switch (fork_pid = fork())
	{
		case -1:
			perror("execute_cmd.fork");
			return -1;
		case 0: /* child */
			cmd_parts = split_cmd(cmd->str);
			if (cmd_parts[0] == NULL) /* empty command, ok */
			{
				free(cmd_parts);
				exit(EXIT_SUCCESS);
			}
			/*close(STDIN_FILENO);
			close(STDOUT_FILENO);*/
			if (input != STDIN_FILENO)
			{
				dup2(input, STDIN_FILENO); /* ... */
				close(input);
			}
			/*close(input);*/
			if (output != STDOUT_FILENO)
			{
				dup2(output, STDOUT_FILENO); /* ... */
				close(output);
			}
			/*close(output);*/
			/*for (i = 1; cmd_parts[i] != NULL; ++i)
			{
				prepare_param(&(cmd_parts[i]));
			}*/
			find_executable(&(cmd_parts[0]));
			execve(cmd_parts[0], cmd_parts, envp);
			perror("execute_cmd.fork.execve");
			for (i = 0; cmd_parts[i] != NULL; ++i)
			{
				free(cmd_parts[i]);
			}
			free(cmd_parts);
			/*free_memory();*/
			free_settings();
			exit(EXIT_EXEC_FAILURE);
		default: /* parent */
			if (input != STDIN_FILENO)
			{
				close(input);
			}
			if (output != STDOUT_FILENO)
			{
				close(output);
			}
			if (wait_child)
			{
				if (waitpid(fork_pid, &status, WUNTRACED | WCONTINUED) == -1)
				{
					perror("execute_cmd.waitpid");
					return -1;
				}
				return WEXITSTATUS(status);
			}
			else
			{
				return 0;
			}
			break;
	}
	return status;
}

static void new_pipe(int ***pipes, int input, int output, size_t *count)
{
	*pipes = (int **)realloc(*pipes, (++*count) * sizeof(int *));
	if (*pipes == NULL)
	{
		perror("execute.realloc");
		exit(EXIT_FAILURE);
	}
	(*pipes)[(*count) - 1] = (int *)malloc(2 * sizeof(int *));
	if ((*pipes)[(*count) - 1] == NULL)
	{
		perror("execute.malloc");
		exit(EXIT_FAILURE);
	}
	(*pipes)[(*count) - 1][0] = input; /* input for next command */
	(*pipes)[(*count) - 1][1] = output; /* output for next command */
}

int execute(struct string *_command, char **environment, int input, int output)
{
	size_t start, end, count = 1;
	int **pipes, break_flag, temp_pipe[2];
	struct string sub_cmd, command;
	char quote/*, return_status*/, wait_flag/*, execute_flag*/;
	enum {OP_AND, OP_OR, OP_OTHER} prev_operator, curr_operator;
	/* ... */
	/* return execute_cmd(command, environment, input, output, 1); */
	/* ... */
	make_string(&sub_cmd);
	make_string(&command);
	copy_string(&command, _command);
	append_string_char(&command, ';');
	pipes = (int **)malloc(sizeof(int *));
	if (pipes == NULL)
	{
		perror("execute.malloc");
		exit(EXIT_FAILURE);
	}
	pipes[0] = (int *)malloc(2 * sizeof(int));
	if (pipes[0] == NULL)
	{
		perror("execute.malloc");
		exit(EXIT_FAILURE);
	}
	pipes[0][0] = input;
	pipes[0][1] = output;
	return_status = 0;
	/*execute_flag = 1;*/
	prev_operator = OP_OTHER;
	wait_flag = 1;
	for (start = 0; start < strlen(command.str); start = end)
	{
		for (end = start; end < strlen(command.str); ++end)
		{
			break_flag = 0;
			switch (command.str[end])
			{
				case '\\':
					++end;
					break;
				/*case '{':
				case '(':
					break;*/ /* they should be replaced earlier */
				case '\'':
				case '"':
					quote = command.str[end];
					while (command.str[++end] != quote);
					break;
				case ';': /* next command with default input and output */
					new_pipe(&pipes, input, output, &count);
					sub_string(&sub_cmd, &command, start, end - start);
					curr_operator = OP_OTHER;
					/*if (execute_flag)
					{
						return_status = execute_cmd(&sub_cmd, environment, pipes[count - 2][0], pipes[count - 1][0], 1);
					}
					execute_flag = 1;*/
					wait_flag = 1;
					break_flag = 1;
					break;
				case '&':
					if (end + 1 == strlen(command.str) || command.str[end + 1] != '&') /* run in background */
					{
						new_pipe(&pipes, input, output, &count);
						sub_string(&sub_cmd, &command, start, end - start);
						curr_operator = OP_OTHER;
						/*if (execute_flag)
						{
							return_status = execute_cmd(&sub_cmd, environment, pipes[count - 2][0], pipes[count - 1][0], 0);
						}
						execute_flag = 1;*/
						wait_flag = 0;
						break_flag = 1;
					}
					if (end + 1 < strlen(command.str) && command.str[end + 1] == '&') /* run if previous was successful */
					{
						new_pipe(&pipes, input, output, &count);
						sub_string(&sub_cmd, &command, start, end - start);
						curr_operator = OP_AND;
						/*if (execute_flag)
						{
							return_status = execute_cmd(&sub_cmd, environment, pipes[count - 2][0], pipes[count - 1][0], 1);
							execute_flag = (return_status == 0);
						}
						else
						{
							execute_flag = 0;
						}*/
						wait_flag = 1;
						break_flag = 1;
						++end;
					}
					break;
				case '|':
					if (end + 1 == strlen(command.str) || command.str[end + 1] != '|') /* run with pipe */
					{
						/* change pipes */
						if (pipe(temp_pipe) == -1)
						{
							perror("execute.pipe");
							exit(EXIT_FAILURE);
						}
						new_pipe(&pipes, temp_pipe[0], output, &count);
						pipes[count - 2][1] = temp_pipe[1];
						/*pipes[count - 1][0] = temp_pipe[0];*/
						sub_string(&sub_cmd, &command, start, end - start);
						curr_operator = OP_OTHER;
						/*if (execute_flag)
						{
							return_status = execute_cmd(&sub_cmd, environment, pipes[count - 2][0], pipes[count - 1][0], 0);
						}
						execute_flag = 1;*/
						wait_flag = 0;
						break_flag = 1;
					}
					if (end + 1 < strlen(command.str) && command.str[end + 1] == '|') /* run if previous was not successful */
					{
						new_pipe(&pipes, input, output, &count);
						sub_string(&sub_cmd, &command, start, end - start);
						curr_operator = OP_OR;
						/*if (execute_flag)
						{
							return_status = execute_cmd(&sub_cmd, environment, pipes[count - 2][0], pipes[count - 1][0], 1);
							execute_flag = (return_status != 0);
						}
						else
						{
							execute_flag = 0;
						}*/
						wait_flag = 1;
						break_flag = 1;
						++end;
					}
					break;
			}
			if (break_flag)
			{
				/*size_t i;
				for (i = 0; i < count; ++i)
				{
					printf("%d %d\n", pipes[i][0], pipes[i][1]);
				}*/
				switch (prev_operator)
				{
					case OP_AND:
						if (return_status == 0)
						{
							return_status = execute_cmd(&sub_cmd, environment, pipes[count - 2][0], pipes[count - 2][1], wait_flag);
						}
						break;
					case OP_OR:
						if (return_status != 0)
						{
							return_status = execute_cmd(&sub_cmd, environment, pipes[count - 2][0], pipes[count - 2][1], wait_flag);
						}
						break;
					default:
						return_status = execute_cmd(&sub_cmd, environment, pipes[count - 2][0], pipes[count - 2][1], wait_flag);
				}
				prev_operator = curr_operator;
				end++;
				break;
			}
		}
	}
	for (start = 0; start < count; ++start)
	{
		/*if (pipes[start][0] != input)
		{
			close(pipes[start][0]);
		}
		if (pipes[start][1] != output)
		{
			close(pipes[start][1]);
		}*/
		free(pipes[start]);
	}
	free(pipes);
	free_string(&command);
	free_string(&sub_cmd);
	return 0;
}

int execute_into_string(struct string *cmd, char **envp, int input, struct string *output)
{
	int result, p[2];
	char c;
	if (pipe(p) == -1)
	{
		perror("execute_into_string.pipe");
		exit(EXIT_FAILURE);
	}
	result = execute(cmd, envp, input, p[1]);
	close(p[1]);
	clear_string(output);
	while (read(p[0], &c, 1) == 1)
	{
		if (c == '\n')
		{
			c = ' ';
		}
		append_string_char(output, c);
	}
	close(p[0]);
	return result;
}
