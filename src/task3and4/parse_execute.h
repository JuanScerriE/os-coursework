#pragma once

#include <stdbool.h>

#include "tokeniser.h"
#include "external.h"
#include "builtin.h"

int parse(token_vec_t toks);
pid_t execute(token_vec_t toks);
