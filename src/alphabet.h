#pragma once

#include <stddef.h>
#include <stdio.h>

#include "transcode.h"

Symbol *read_alphabet(FILE *f, size_t *count);
Symbol *make_bytes_alphabet(size_t *count);
void    free_alphabet(Symbol *syms, size_t count);
