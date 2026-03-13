#include "alphabet.h"

#include <stdlib.h>
#include <string.h>

/*
 * Read an alphabet file (one symbol per line) into a heap-allocated array of
 * Symbol structs.  Each sym buffer is separately heap-allocated.
 * Sets *count on success.  Returns NULL on error.
 */
Symbol *read_alphabet(FILE *f, size_t *count) {
  if (fseek(f, 0, SEEK_END) != 0) return NULL;
  long sz = ftell(f);
  if (sz < 0) return NULL;
  rewind(f);

  char *buf = malloc((size_t)sz + 1);
  if (!buf) return NULL;
  size_t n = fread(buf, 1, (size_t)sz, f);
  buf[n] = '\0';

  /* Count newline-delimited lines (handle missing final newline) */
  size_t nlines = 0;
  for (size_t i = 0; i < n; i++)
    if (buf[i] == '\n') nlines++;
  if (n > 0 && buf[n - 1] != '\n') nlines++;

  Symbol *syms = malloc(nlines * sizeof(*syms));
  if (!syms) { free(buf); return NULL; }

  size_t idx  = 0;
  char  *line = buf;
  for (size_t i = 0; i <= n; i++) {
    if (i == n || buf[i] == '\n') {
      size_t end = i;
      if (end > (size_t)(line - buf) && buf[end - 1] == '\r') end--; /* strip \r */
      size_t sym_len = (size_t)(buf + end - line);
      if (i == n && sym_len == 0) break; /* ignore empty trailing line */

      char *sym = malloc(sym_len);
      if (!sym) {
        for (size_t j = 0; j < idx; j++) free((char *)syms[j].sym);
        free(syms); free(buf);
        return NULL;
      }
      memcpy(sym, line, sym_len);
      syms[idx++] = (Symbol){ sym, sym_len };
      line = buf + i + 1;
    }
  }

  free(buf);
  *count = idx;
  return syms;
}

/* Build a 256-symbol raw-bytes alphabet where symbol[i] is the single byte
 * with value i.  Each symbol carries an explicit length of 1 so NUL bytes
 * are handled correctly. */
Symbol *make_bytes_alphabet(size_t *count) {
  Symbol *syms = malloc(256 * sizeof(*syms));
  if (!syms) return NULL;
  for (int i = 0; i < 256; i++) {
    char *buf = malloc(1);
    if (!buf) {
      for (int j = 0; j < i; j++) free((char *)syms[j].sym);
      free(syms);
      return NULL;
    }
    buf[0] = (char)i;
    syms[i] = (Symbol){ buf, 1 };
  }
  *count = 256;
  return syms;
}

void free_alphabet(Symbol *syms, size_t count) {
  for (size_t i = 0; i < count; i++) free((char *)syms[i].sym);
  free(syms);
}
