#include "cd.h"

int cd(char *path)
{
	if (path == NULL)
	{
		
	}
	else
	{
		if (chdir(path) == -1)
		{
			perror("cd.chrid");
			return -1;
		}
	}
	return 0;
}
