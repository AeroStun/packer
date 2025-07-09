#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <file> <offset>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *const filename = argv[1];
  const long offset = strtol(argv[2], NULL, 0);

  FILE *f = fopen(filename, "r+b");
  if (!f) {
    perror("fopen");
    return EXIT_FAILURE;
  }

  // Get file size
  if (fseek(f, 0, SEEK_END) != 0) {
    perror("fseek to end");
    fclose(f);
    return EXIT_FAILURE;
  }

  const long size = ftell(f);
  if (size < 0) {
    perror("ftell");
    fclose(f);
    return EXIT_FAILURE;
  }

  const uint64_t size64 = (uint64_t)size;

  if (fseek(f, offset, SEEK_SET) != 0) {
    perror("fseek to offset");
    return EXIT_FAILURE;
  }

  printf("Offset: %d\nSize: %d\n", (int)offset, (int)size);

  if (fwrite(&size64, sizeof(size64), 1, f) != 1) {
    perror("fwrite");
    return EXIT_FAILURE;
  }

  fclose(f);
  return EXIT_SUCCESS;
}
