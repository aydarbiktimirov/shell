#include "stack.h"

void make_stack(struct stack *stack, size_t size)
{
	stack->element_size = size;
	stack->head = NULL;
}

void push_stack(struct stack *stack, void *data)
{
	struct stack_element *element;
	element = (struct stack_element *)malloc(sizeof(struct stack_element));
	if (element == NULL)
	{
		perror("push_stack.malloc");
		exit(EXIT_FAILURE);
	}
	element->data = malloc(stack->element_size);
	if (element == NULL)
	{
		perror("push_stack.malloc");
		exit(EXIT_FAILURE);
	}
	memcpy(element->data, data, stack->element_size);
	element->next = stack->head;
	stack->head = element;
}

void pop_stack(struct stack *stack, void *dest)
{
	struct stack_element *new_head;
	if (stack->head == NULL)
	{
		return;
	}
	if (dest != NULL)
	{
		memcpy(dest, stack->head->data, stack->element_size);
	}
	free(stack->head->data);
	new_head = stack->head->next;
	free(stack->head);
	stack->head = new_head;
}

void clear_stack(struct stack *stack)
{
	while (!empty_stack(stack))
	{
		pop_stack(stack, NULL);
	}
}

char empty_stack(struct stack *stack)
{
	return stack->head == NULL ? 1 : 0;
}

void reverse_stack(struct stack *stack)
{
	struct stack_element *a, *b, *c;
	if (stack->head == NULL || stack->head->next == NULL)
	{
		return;
	}
	a = stack->head;
	b = a->next;
	c = b->next;
	for (a->next = NULL; c != NULL; c = c->next)
	{
		b->next = a;
		a = b;
		b = c;
	}
	b->next = a;
	stack->head = b;
}

void free_stack(struct stack *stack)
{
	clear_stack(stack);
}
