#include "transcode.h"

#include <stdlib.h>
#include <string.h>

/*
 * transcode - re-encode data from one alphabet/base to another.
 *
 * The input bytes are parsed greedily: at each byte offset the first symbol
 * in src_alpha (by index order) whose bytes match the input at that position
 * is consumed.  The resulting sequence of digit values is treated as a
 * big-endian integer in base src_alpha_len, which is then expressed in base
 * dst_alpha_len and each digit mapped to the corresponding symbol in dst_alpha.
 *
 * Leading zero-valued input symbols (index 0 in src_alpha) are preserved as
 * leading first-symbols of dst_alpha so that encodings with meaningful leading
 * zeros round-trip correctly.
 *
 * Parameters:
 *   input         - input bytes to transcode (need not be NUL-terminated)
 *   input_len     - number of input bytes
 *   src_alpha     - ordered symbols of the source base; index == digit value
 *   src_alpha_len - number of symbols in the source alphabet (>= 2)
 *   dst_alpha     - ordered symbols of the destination base
 *   dst_alpha_len - number of symbols in the destination alphabet (>= 2)
 *   out_len       - set to the number of output bytes on success
 *
 * Returns a heap-allocated, NUL-terminated output buffer, or NULL on error
 * (unknown symbol, invalid arguments, or allocation failure).
 * The caller is responsible for freeing the returned buffer.
 */
char *transcode(
  const char   *input,     size_t input_len,
  const Symbol *src_alpha, size_t src_alpha_len,
  const Symbol *dst_alpha, size_t dst_alpha_len,
  size_t *out_len
) {
  if (!input || !src_alpha || !dst_alpha || !out_len) return NULL;
  if (src_alpha_len < 2 || dst_alpha_len < 2)         return NULL;

  /* --- 1. Greedy first-match parse: input bytes -> digit values ---------- */

  /* At most input_len digits (achieved when every symbol is 1 byte) */
  unsigned int *digits = malloc(input_len * sizeof(*digits));
  if (!digits) return NULL;

  size_t num_digits = 0;
  size_t leading    = 0; /* count of leading zero-value (index 0) symbols */
  size_t pos        = 0;

  while (pos < input_len) {
    int found = 0;
    for (size_t i = 0; i < src_alpha_len; i++) {
      size_t sym_len = src_alpha[i].len;
      if (sym_len > 0 &&
          pos + sym_len <= input_len &&
          memcmp(input + pos, src_alpha[i].sym, sym_len) == 0) {
        if (num_digits == leading && i == 0) leading++;
        digits[num_digits++] = (unsigned int)i;
        pos += sym_len;
        found = 1;
        break;
      }
    }
    if (!found) { free(digits); return NULL; }
  }

  /* --- 2. Base-convert: repeatedly divide by dst_alpha_len ------------- *
   *                                                                        *
   * Each iteration does one long-division of the big-endian digit array   *
   * (base src_alpha_len) by dst_alpha_len.  The remainder is the next     *
   * output digit (LSB first); the quotient replaces the array.            *
   *                                                                        *
   * Output capacity upper bound: num_digits * ceil(log2(src_alpha_len))+1 *
   * (each src digit contributes at most ceil(log2(src)) bits; each dst    *
   * digit carries at least 1 bit since dst >= 2).                         */

  size_t bits = 0;
  for (size_t v = src_alpha_len - 1; v > 0; v >>= 1) bits++;
  if (bits == 0) bits = 1;
  size_t out_cap = num_digits * bits + 1;

  unsigned int *out_digits = calloc(out_cap, sizeof(*out_digits));
  if (!out_digits) { free(digits); return NULL; }

  size_t out_size = 0;
  size_t start    = 0; /* index of the first non-zero working digit */

  while (start < num_digits) {
    unsigned long rem       = 0;
    size_t        new_start = num_digits; /* sentinel: all-zero quotient */

    for (size_t i = start; i < num_digits; i++) {
      unsigned long cur = rem * (unsigned long)src_alpha_len + digits[i];
      digits[i] = (unsigned int)(cur / dst_alpha_len);
      rem       = cur % dst_alpha_len;
      if (new_start == num_digits && digits[i] != 0) new_start = i;
    }

    out_digits[out_size++] = (unsigned int)rem;
    start = new_start; /* skip newly-created leading zeros next round */
  }

  free(digits);

  /* --- 3. Build the output --------------------------------------------- */

  /* Compute total output byte length */
  size_t dst0_len = dst_alpha[0].len;
  size_t total    = leading * dst0_len;
  for (size_t i = 0; i < out_size; i++)
    total += dst_alpha[out_digits[out_size - 1 - i]].len;

  *out_len = total;
  char *result = malloc(total + 1);
  if (!result) { free(out_digits); return NULL; }

  /* Preserve leading zero-value symbols as the first dst symbol */
  size_t off = 0;
  for (size_t i = 0; i < leading; i++) {
    memcpy(result + off, dst_alpha[0].sym, dst0_len);
    off += dst0_len;
  }

  /* out_digits is LSB-first; write MSB-first into result */
  for (size_t i = 0; i < out_size; i++) {
    const Symbol *s = &dst_alpha[out_digits[out_size - 1 - i]];
    memcpy(result + off, s->sym, s->len);
    off += s->len;
  }

  result[total] = '\0';
  free(out_digits);
  return result;
}
