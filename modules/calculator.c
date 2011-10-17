#include "calculator.h"

static char left_operator(struct calc_token t)
{
	return t.value.operator < CALC_POWER;
}

static char right_operator(struct calc_token t)
{
	return t.value.operator == CALC_POWER;
}

static char precedence_operator(struct calc_token t)
{
	switch (t.value.operator)
	{
		case CALC_POWER:
			return 3;
		case CALC_MULT:
		case CALC_DIV:
		case CALC_REST:
			return 2;
		default:
			return 1;
	}
}

static char calculateRPN(struct stack *s, int *result)
{
	struct stack temp;
	struct calc_token element;
	int o1 = 0, o2 = 0;
	*result = 0;
	make_stack(&temp, sizeof(int));
	while (!empty_stack(s))
	{
		pop_stack(s, &element);
		if (element.type == CALC_NUMBER)
		{
			push_stack(&temp, &(element.value.value));
			*result = element.value.value;
		}
		else
		{
			pop_stack(&temp, &o1);
			pop_stack(&temp, &o2);
			switch (element.value.operator)
			{
				case CALC_ADD:
					*result = o2 + o1;
					break;
				case CALC_SUB:
					*result = o2 - o1;
					break;
				case CALC_DIV:
					if (o1 == 0)
					{
						free_stack(&temp);
						return -1;
					}
					*result = o2 / o1;
					break;
				case CALC_REST:
					*result = o2 % o1;
					break;
				case CALC_MULT:
					*result = o2 * o1;
					break;
				case CALC_POWER:
					*result = (int)pow(o2, o1);
					break;
			}
			push_stack(&temp, result);
		}
	}
	free_stack(&temp);
	return 0;
}

char calculate(char *s, int *dest)
{
	struct stack result, operators;
	struct calc_token token, temp;
	size_t i;
	make_stack(&result, sizeof(struct calc_token));
	make_stack(&operators, sizeof(struct calc_token));
	for (i = 0; i < strlen(s); )
	{
		for (; i < strlen(s) && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n'); ++i); /* skip spaces */
		if (i == strlen(s))
		{
			break;
		}
		if (s[i] >= '0' && s[i] <= '9') /* number */
		{
			token.type = CALC_NUMBER;
			token.value.value = 0;
			for (; i < strlen(s) && s[i] >= '0' && s[i] <= '9'; ++i)
			{
				token.value.value *= 10;
				token.value.value += (s[i] - '0');
			}
			push_stack(&result, &token);
		}
		else
		{
			token.type = CALC_OPERATOR;
			switch (s[i])
			{
				case '+':
					token.value.operator = CALC_ADD;
					break;
				case '-':
					token.value.operator = CALC_SUB;
					break;
				case '/':
					token.value.operator = CALC_DIV;
					break;
				case '%':
					token.value.operator = CALC_REST;
					break;
				case '*':
					if (i + 1 < strlen(s) && s[i + 1] == '*')
					{
						token.value.operator = CALC_POWER;
						++i;
					}
					else
					{
						token.value.operator = CALC_MULT;
					}
					break;
				case '(':
					token.type = CALC_OPEN;
					break;
				case ')':
					token.type = CALC_CLOSE;
					break;
				default:
					free_stack(&operators);
					free_stack(&result);
					return -1;
			}
			++i;
			if (token.type == CALC_OPERATOR)
			{
				while (!empty_stack(&operators))
				{
					pop_stack(&operators, &temp);
					if (temp.type == CALC_OPERATOR && ((left_operator(token) && precedence_operator(token) <= precedence_operator(temp)) || (right_operator(token) && precedence_operator(token) < precedence_operator(temp))))
					{
						push_stack(&result, &temp);
					}
					else
					{
						push_stack(&operators, &temp);
						break;
					}
				}
				push_stack(&operators, &token);
			}
			else
			{
				if (token.type == CALC_OPEN)
				{
					push_stack(&operators, &token);
				}
				else
				{
					while (!empty_stack(&operators))
					{
						pop_stack(&operators, &temp);
						if (empty_stack(&operators) && temp.type != CALC_OPEN)
						{
							free_stack(&operators);
							free_stack(&result);
							return -1;
						}
						if (temp.type == CALC_OPEN)
						{
							break;
						}
						push_stack(&result, &temp);
					}
				}
			}
		}
	}
	while (!empty_stack(&operators))
	{
		pop_stack(&operators, &temp);
		if (temp.type == CALC_OPEN)
		{
			free_stack(&operators);
			free_stack(&result);
			return -1;
		}
		push_stack(&result, &temp);
	}
	reverse_stack(&result);
	if (calculateRPN(&result, dest) != 0)
	{
		free_stack(&operators);
		free_stack(&result);
		return -1;
	}
	free_stack(&operators);
	free_stack(&result);
	return 0;
}
