#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage(const char *prog) {
  fprintf(stderr, "Usage: %s [-s src_alphabet] [-d dst_alphabet] <input> <output>\n", prog);
  fprintf(stderr, "  -s src_alphabet  File path of the source encoding alphabet (default: bytes)\n");
  fprintf(stderr, "  -d dst_alphabet  File path of the destination encoding alphabet (default: bytes)\n");
  fprintf(stderr, "  input            Input file path\n");
  fprintf(stderr, "  output           Output file path\n");
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

  FILE *in = fopen(input_path, "r");
  if (!in) {
    perror(input_path);
    return EXIT_FAILURE;
  }

  FILE *out = fopen(output_path, "w");
  if (!out) {
    perror(output_path);
    fclose(in);
    return EXIT_FAILURE;
  }

  FILE *src_alpha = NULL;
  if (src_alphabet_path) {
    src_alpha = fopen(src_alphabet_path, "r");
    if (!src_alpha) {
      perror(src_alphabet_path);
      fclose(in);
      fclose(out);
      return EXIT_FAILURE;
    }
  }

  FILE *dst_alpha = NULL;
  if (dst_alphabet_path) {
    dst_alpha = fopen(dst_alphabet_path, "r");
    if (!dst_alpha) {
      perror(dst_alphabet_path);
      if (src_alpha) fclose(src_alpha);
      fclose(in);
      fclose(out);
      return EXIT_FAILURE;
    }
  }

  /* TODO: implement encoding translation using src_alpha, dst_alpha, in, out */
  /* NULL src_alpha or dst_alpha means use raw bytes as the alphabet          */
  (void)src_alpha;
  (void)dst_alpha;
  (void)in;
  (void)out;

  if (src_alpha) fclose(src_alpha);
  if (dst_alpha) fclose(dst_alpha);
  fclose(in);
  fclose(out);

  return EXIT_SUCCESS;
}
