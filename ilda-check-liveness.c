#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ilda-decoder.h"

static ssize_t read_file(void *opaque, void *buffer, size_t len) {
  return read((int)(long)opaque, buffer, len);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: ilda-check-liveness FILE pps\n");
    fprintf(
        stderr,
        "  Use strict mode if ILDA_STRICT_MODE environment variable exists\n");
    fprintf(stderr, "  You can use the `k' suffix such as 30k\n");
    exit(1);
  }
  int f = open(argv[1], 0);
  if (f < 0) {
    perror("cannot open file");
    exit(1);
  }
  char *endptr;
  size_t pps = strtol(argv[2], &endptr, 10);
  if (*endptr == 'k') {
    pps *= 1000;
    endptr++;
  }
  if (*argv[2] == '\0' || *endptr != '\0') {
    fprintf(stderr, "incorrect PPS specification %s\n", argv[2]);
    exit(1);
  }
  ilda_state_t ilda;
  ilda_init(&ilda, read_file, (void *)(long)f,
            getenv("ILDA_STRICT_MODE") != NULL);
  ilda_pos_t current_position = {0, 0, 0};
  size_t current_position_start = 0;
  size_t max_length = 0;
  size_t current_index = 0;
  for (;;) {
    const ilda_header_t *header = ilda_read_next_header(&ilda);
    if (header == NULL) {
      fprintf(stderr, "error when reading ILDA file: %s\n", ilda.error);
      exit(1);
    }
    if (ilda_is_end_of_file(header)) {
      break;
    }
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
      for (uint16_t i = 0; i < header->number_of_records; i++) {
        if (ilda_is_blanking(buffer[i].status_code) ||
            buffer[i].pos.x != current_position.x ||
            buffer[i].pos.y != current_position.y ||
            buffer[i].pos.z != current_position.z) {
          size_t length = current_index - current_position_start;
          if (length > max_length)
            max_length = length;
          current_position.x = buffer[i].pos.x;
          current_position.y = buffer[i].pos.y;
          current_position.z = buffer[i].pos.z;
          current_position_start = current_index;
        }
        current_index += 1;
      }
    }
  }
  printf("Longest non-moving sequence: %lu points (%.1f ms)\n", max_length,
         max_length * 1000.0 / pps);
}
