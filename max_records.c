#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "ilda_decoder.h"

static ssize_t read_file(void *opaque, void *buffer, size_t len) {
  return read((int) (long) opaque, buffer, len);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: max_records FILE\n");
    fprintf(stderr, "  Use strict mode if ILDA_STRICT_MODE environment variable exists\n");
    exit(1);
  }
  int f = open(argv[1], 0);
  if (f < 0) {
    perror("cannot open file");
    exit(1);
  }
  ilda_state_t ilda;
  ilda_init(&ilda, read_file, (void *) (long) f, getenv("ILDA_STRICT_MODE") != NULL);
  uint16_t max_records = 0;
  for (;;) {
    const ilda_header_t *header = ilda_read_next_header(&ilda);
    if (header == NULL) {
      fprintf(stderr, "error when reading ILDA file: %s\n", ilda.error);
      exit(1);
    }
    if (ilda_is_end_of_file(header)) {
      break;
    }
    if (header->number_of_records > max_records)
      max_records = header->number_of_records;
    if (ilda_is_palette(header)) {
      if (ilda_read_palette(&ilda)) {
        fprintf(stderr, "error when reading palette: %s\n", ilda.error);
        exit(1);
      }
    } else {
      static ilda_point_t buffer[65536];
      if (ilda_read_records(&ilda, buffer, sizeof buffer)) {
        fprintf(stderr, "error when reading records: %s\n", ilda.error);
        exit(1);
      }
    }
  }
  printf("Maximum number of records in a frame: %u\n", max_records);
}
