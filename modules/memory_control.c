#include "memory_control.h"
#undef malloc
#undef realloc
#undef free

static size_t count = 0, size = 1;
static void **memory = NULL;

static void *add_block(void *data)
{
	if (memory == NULL)
	{
		if ((memory = (void **)malloc(size * sizeof(void *))) == NULL)
		{
			perror("add_block.malloc");
			exit(EXIT_FAILURE);
		}
	}
	memory[count++] = data;
	if (count == size)
	{
		size *= 2;
		if ((memory = (void **)realloc(memory, size * sizeof(void *))) == NULL)
		{
			perror("add_block.realloc");
			exit(EXIT_FAILURE);
		}
	}
	return data;
}

static void *remove_block(void *block)
{
	size_t i;
	void *res;
	for (i = 0; i < count; ++i)
	{
		if (memory[i] == block)
		{
			break;
		}
	}
	if (i == count)
	{
		return block;
	}
	res = memory[i];
	memory[i] = memory[--count];
	return res;
}

void free_memory()
{
	size_t i;
	printf("%d:\n", (int)count);
	for (i = 0; i < count; ++i)
	{
		printf("%d\n", (int)i);
		fflush(stdout);
		free(memory[i]);
	}
	free(memory);
}

void *_malloc(size_t size)
{
	return add_block(malloc(size));
}

void *_realloc(void *block, size_t size)
{
	if (block == NULL)
	{
		return add_block(realloc(block, size));
	}
	return realloc(block, size);
}

void _free(void *block)
{
	free(remove_block(block));
}
