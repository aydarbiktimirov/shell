GCC=gcc
OPTIONS=-g -Wall -Wextra -pedantic -lm -lcurses

all:
	$(GCC) $(OPTIONS) {shell,modules,.}/*.c -o bin/shell

clean:
	rm $(SHELL_O) $(MODULES_O) bin/shell
