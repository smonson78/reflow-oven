#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define BUFFER_SIZE 4096

// The number of bytes to align to
int target_alignment = 2;

int main(int argc, char **argv) {
  printf("Binary object padder\n");

  if (argc != 3) {
    fprintf(stderr, "usage: %s infile outfile\n", argv[0]);
    exit(1);
  }

  FILE *infile = fopen(argv[1], "rb");
  if (infile == NULL) {
    fprintf(stderr, "Couldn't open input file %s!\n", argv[1]);
    exit(2);
  }

  FILE *outfile = fopen(argv[2], "wb");
  if (outfile == NULL) {
    fprintf(stderr, "Couldn't open output file %s!\n", argv[2]);
    exit(2);
  }

  fseeko(infile, 0, SEEK_END);
  size_t size = ftello(infile);
  printf("Size of file is %zd bytes\n", size);
  fseeko(infile, 0, SEEK_SET);

  // Padding needed to bring it to the next 16 bits:
  size_t padding = size % target_alignment;
  padding = padding > 0 ? target_alignment - padding : 0;
  printf("%zd padding bytes needed.\n", padding);

  // Copy block by block
  uint8_t buffer[BUFFER_SIZE];
  while (size > BUFFER_SIZE) {
    fread(buffer, BUFFER_SIZE, 1, infile);
    fwrite(buffer, BUFFER_SIZE, 1, outfile);
    size -= BUFFER_SIZE;
  }

  // Then do the leftover bit, if any
  if (size > 0) {
    fread(buffer, size, 1, infile);
    fwrite(buffer, size, 1, outfile);
  }

  // Then pad the output file to the next 16-bit size
  if (padding > 0) {
    buffer[0] = 0;
    fwrite(buffer, padding, 1, outfile);
  }

  printf("Completed.\n");

  fclose(infile);
  fclose(outfile);
  return 0;
}