#pragma once
#include "memory_control.h"
#include <malloc.h>
#include <string.h>

struct string
{
	char *str;
	size_t size;
};

void string(struct string *, char *);
void make_string(struct string *);
int compare_string(struct string *, struct string *);
void clear_string(struct string *);
void append_string(struct string *, struct string *);
void append_string_char(struct string *, char);
void reverse_string(struct string *);
void insert_string_first_char(struct string *, char);
void sub_string(struct string *, struct string *, size_t, size_t);
void replace_string(struct string *, struct string *, struct string *, struct string *);
void replace_string_part(struct string *, struct string *, struct string *, size_t, size_t);
void delete_string(struct string *, struct string *, struct string *);
void copy_string(struct string *, struct string *);
void free_string(struct string *);
