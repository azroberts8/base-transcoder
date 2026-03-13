#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "alphabet.h"
#include "transcode.h"

static void usage(const char *prog) {
  fprintf(stderr, "Usage: %s [-s src_alphabet] [-d dst_alphabet] <input> <output>\n", prog);
  fprintf(stderr, "  -s src_alphabet  File path of the source encoding alphabet (default: bytes)\n");
  fprintf(stderr, "  -d dst_alphabet  File path of the destination encoding alphabet (default: bytes)\n");
  fprintf(stderr, "  input            Input file path\n");
  fprintf(stderr, "  output           Output file path\n");
}

/* Read an entire file into a heap-allocated buffer, sets *len. */
static char *read_file(FILE *f, size_t *len) {
  if (fseek(f, 0, SEEK_END) != 0) return NULL;
  long sz = ftell(f);
  if (sz < 0) return NULL;
  rewind(f);

  char *buf = malloc((size_t)sz + 1);
  if (!buf) return NULL;
  *len = fread(buf, 1, (size_t)sz, f);
  buf[*len] = '\0';
  return buf;
}

int main(int argc, char *argv[]) {
  const char *src_alphabet_path = NULL;
  const char *dst_alphabet_path = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "s:d:")) != -1) {
    switch (opt) {
      case 's': src_alphabet_path = optarg; break;
      case 'd': dst_alphabet_path = optarg; break;
      default:
        usage(argv[0]);
        return EXIT_FAILURE;
    }
  }

  if (argc - optind != 2) {
    fprintf(stderr, "%s: expected 2 positional arguments, got %d\n", argv[0], argc - optind);
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  const char *input_path  = argv[optind];
  const char *output_path = argv[optind + 1];

  /* --- Open files -------------------------------------------------------- */

  FILE *in = fopen(input_path, "rb");
  if (!in) { perror(input_path); return EXIT_FAILURE; }

  FILE *out = fopen(output_path, "wb");
  if (!out) { perror(output_path); fclose(in); return EXIT_FAILURE; }

  FILE *src_alpha_file = NULL;
  if (src_alphabet_path) {
    src_alpha_file = fopen(src_alphabet_path, "r");
    if (!src_alpha_file) {
      perror(src_alphabet_path);
      fclose(in); fclose(out);
      return EXIT_FAILURE;
    }
  }

  FILE *dst_alpha_file = NULL;
  if (dst_alphabet_path) {
    dst_alpha_file = fopen(dst_alphabet_path, "r");
    if (!dst_alpha_file) {
      perror(dst_alphabet_path);
      if (src_alpha_file) fclose(src_alpha_file);
      fclose(in); fclose(out);
      return EXIT_FAILURE;
    }
  }

  /* --- Build alphabets -------------------------------------------------- */

  int rc = EXIT_SUCCESS;

  size_t  src_alpha_len = 0;
  Symbol *src_alphabet  = src_alpha_file
    ? read_alphabet(src_alpha_file, &src_alpha_len)
    : make_bytes_alphabet(&src_alpha_len);
  if (src_alpha_file) fclose(src_alpha_file);
  if (!src_alphabet) {
    fprintf(stderr, "failed to load source alphabet\n");
    rc = EXIT_FAILURE; goto cleanup_files;
  }

  size_t  dst_alpha_len = 0;
  Symbol *dst_alphabet  = dst_alpha_file
    ? read_alphabet(dst_alpha_file, &dst_alpha_len)
    : make_bytes_alphabet(&dst_alpha_len);
  if (dst_alpha_file) fclose(dst_alpha_file);
  if (!dst_alphabet) {
    fprintf(stderr, "failed to load destination alphabet\n");
    rc = EXIT_FAILURE; goto cleanup_src;
  }

  /* --- Read input -------------------------------------------------------- */

  size_t input_len = 0;
  char  *input     = read_file(in, &input_len);
  if (!input) {
    fprintf(stderr, "failed to read input\n");
    rc = EXIT_FAILURE; goto cleanup_alpha;
  }

  /* --- Transcode --------------------------------------------------------- */

  size_t output_len = 0;
  char  *output     = transcode(
    input, input_len,
    src_alphabet, src_alpha_len,
    dst_alphabet, dst_alpha_len,
    &output_len
  );
  free(input);

  if (!output) {
    fprintf(stderr, "transcoding failed\n");
    rc = EXIT_FAILURE; goto cleanup_alpha;
  }

  if (fwrite(output, 1, output_len, out) != output_len) {
    perror(output_path);
    rc = EXIT_FAILURE;
  }
  free(output);

cleanup_alpha:
  free_alphabet(dst_alphabet, dst_alpha_len);
cleanup_src:
  free_alphabet(src_alphabet, src_alpha_len);
cleanup_files:
  fclose(in);
  fclose(out);
  return rc;
}
