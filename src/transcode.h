#pragma once

#include <stddef.h>

typedef struct {
  const char *sym;
  size_t      len;
} Symbol;

char *transcode(
  const char   *input,      size_t input_len,
  const Symbol *src_alpha,  size_t src_alpha_len,
  const Symbol *dst_alpha,  size_t dst_alpha_len,
  size_t *out_len
);
