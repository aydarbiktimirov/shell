#pragma once
#include "memory_control.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

struct stack_element
{
	void *data;
	struct stack_element *next;
};

struct stack
{
	size_t element_size;
	struct stack_element *head;
};

void make_stack(struct stack *, size_t);
void push_stack(struct stack *, void *);
void pop_stack(struct stack *, void *);
void clear_stack(struct stack *);
char empty_stack(struct stack *);
void reverse_stack(struct stack *);
void free_stack(struct stack *);
