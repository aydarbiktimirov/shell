#include "modules/memory_control.h"
#include "modules/string.h"
#include "modules/calculator.h"
#include "shell/execute.h"
#include "shell/shell.h"
#include <stdlib.h>

int main(int argc, char **argv, char **envp)
{
	set_settings_file(argc > 2 && strcmp(argv[1], "--config") == 0 ? argv[2] : DEFAULT_CONFIG);
	start_shell(envp);
	free_settings();
	return EXIT_SUCCESS;
}
