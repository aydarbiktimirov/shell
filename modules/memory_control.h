#pragma once
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

void *_malloc(size_t);
void *_realloc(void *, size_t);
void _free(void *);
void free_memory();

#define malloc _malloc
#define realloc _realloc
#define free _free
