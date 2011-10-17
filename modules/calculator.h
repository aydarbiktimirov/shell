#pragma once
#include "memory_control.h"
#include "stack.h"
#include <math.h>


struct calc_token
{
	enum
	{
		CALC_NUMBER = 0, CALC_OPERATOR, CALC_OPEN, CALC_CLOSE
	} type;
	union
	{
		int value;
		enum
		{
			CALC_ADD = 0, CALC_SUB, CALC_DIV, CALC_REST, CALC_MULT, CALC_POWER
		} operator;
	} value;
};

char calculate(char *, int *);
