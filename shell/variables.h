#pragma once
#include "../modules/memory_control.h"
#include <sys/types.h>
#include <pwd.h>
#include "../modules/string.h"
#include "../modules/settings.h"
#include "../shell/execute.h"

void set_variable(struct string *, struct string *);
void get_variable(struct string *, struct string *);
