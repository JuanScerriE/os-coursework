#pragma once

#include <stdbool.h>

#include "builtin.h"
#include "external.h"
#include "tokeniser.h"

int parse(token_vec_t toks);
pid_t execute(token_vec_t toks);
