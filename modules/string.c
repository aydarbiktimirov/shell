#include "string.h"

void string(struct string *dest, char *str)
{
	size_t i;
	if (dest->str != NULL)
	{
		clear_string(dest);
	}
	for (i = 0; str[i] != 0; ++i)
	{
		append_string_char(dest, str[i]);
	}
}

static void normalize_string_length(struct string *s)
{
	if (s->size / 2 == 0)
	{
		return;
	}
	if (strlen(s->str) < s->size / 4)
	{
		s->size /= 2;
		s->str = (char *)realloc(s->str, s->size * sizeof(char));
		normalize_string_length(s);
	}
}

void make_string(struct string *s)
{
	s->str = NULL;
	clear_string(s);
}

int compare_string(struct string *s1, struct string *s2)
{
	return strcmp(s1->str, s2->str);
}

void clear_string(struct string *s)
{
	s->str = (char *)realloc(s->str, sizeof(char));
	s->str[0] = 0;
	s->size = 1;
}

void append_string(struct string *dest, struct string *s)
{
	if (strlen(dest->str) + strlen(s->str) >= dest->size)
	{
		dest->size = 2 * (dest->size > s->size ? dest->size : s->size);
		dest->str = (char *)realloc(dest->str, dest->size * sizeof(char));
	}
	strcat(dest->str, s->str);
	normalize_string_length(dest);
}

void append_string_char(struct string *dest, char c)
{
	if (strlen(dest->str) + 1 == dest->size)
	{
		dest->size *= 2;
		dest->str = (char *)realloc(dest->str, dest->size * sizeof(char));
	}
	strncat(dest->str, &c, 1);
}

void reverse_string(struct string *str)
{
	size_t i;
	char tmp;
	for (i = 0; i < strlen(str->str) / 2; ++i)
	{
		tmp = str->str[i];
		str->str[i] = str->str[strlen(str->str) - i - 1];
		str->str[strlen(str->str) - i - 1] = tmp;
	}
}

void insert_string_first_char(struct string *dest, char c)
{
	struct string temp;
	make_string(&temp);
	copy_string(&temp, dest);
	/*if (strlen(dest->str) + 1 == dest->size)
	{
		dest->size *= 2;
		dest->str = (char *)realloc(dest->str, dest->size * sizeof(char));
	}
	strcpy(&(dest->str[1]), dest->str);
	dest->str[0] = c;*/
	clear_string(dest);
	append_string_char(dest, c);
	append_string(dest, &temp);
	free_string(&temp);
}

void sub_string(struct string *dest, struct string *_s, size_t from, size_t length)
{
	struct string s;
	if (from >= strlen(_s->str))
	{
		clear_string(dest);
		return;
	}
	make_string(&s);
	copy_string(&s, _s);
	if (strlen(s.str) - from < length)
	{
		length = strlen(s.str) - from;
	}
	if (length >= dest->size)
	{
		dest->size = s.size;
		dest->str = (char *)realloc(dest->str, dest->size * sizeof(char));
	}
	memset(dest->str, 0, dest->size * sizeof(char));
	strncpy(dest->str, &(s.str[from]), length);
	normalize_string_length(dest);
	free_string(&s);
}

void replace_string(struct string *dest, struct string *_s, struct string *replace, struct string *subject)
{
	size_t i, j, break_flag;
	struct string temp, s;
	make_string(&s);
	copy_string(&s, _s);
	for (i = 0; i <= strlen(s.str) - strlen(replace->str); ++i)
	{
		break_flag = 0;
		for (j = 0; j < strlen(replace->str); ++j)
		{
			if (s.str[i + j] != replace->str[j])
			{
				break_flag = 1;
				break;
			}
		}
		if (break_flag == 0)
		{
			break;
		}
	}
	if (break_flag == 1)
	{
		free_string(&s);
		return;
	}
	clear_string(dest);
	make_string(&temp);
	sub_string(&temp, &s, 0, i);
	append_string(dest, &temp);
	if (subject != NULL)
	{
		append_string(dest, subject);
	}
	sub_string(&temp, &s, i + strlen(replace->str), s.size);
	append_string(dest, &temp);
	free_string(&temp);
	normalize_string_length(dest);
	free_string(&s);
}

void replace_string_part(struct string *dest, struct string *s, struct string *replace, size_t start, size_t end)
{
	struct string result, tmp;
	if (start > strlen(s->str))
	{
		start = strlen(s->str);
	}
	if (end > strlen(s->str))
	{
		end = strlen(s->str);
	}
	if (start > end)
	{
		return;
	}
	make_string(&result);
	make_string(&tmp);
	sub_string(&result, s, 0, start);
	append_string(&result, replace);
	sub_string(&tmp, s, end, strlen(s->str));
	append_string(&result, &tmp);
	copy_string(dest, &result);
	free_string(&tmp);
	free_string(&result);
	
}

void delete_string(struct string *dest, struct string *s, struct string *x)
{
	replace_string(dest, s, x, NULL);
}

void copy_string(struct string *dest, struct string *source)
{
	/*memset(dest->str, 0, dest->size * sizeof(char));*/
	dest->size = source->size;
	dest->str = (char *)realloc(dest->str, dest->size * sizeof(char));
	strcpy(dest->str, source->str);
}

void free_string(struct string *s)
{
	free(s->str);
	s->str = NULL;
	s->size = 1;
}
