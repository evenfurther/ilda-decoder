#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "ilda-decoder.h"

static ssize_t read_file(void *opaque, void *buffer, size_t len) {
  return read((int) (long) opaque, buffer, len);
}

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: ilda-dump FILE\n");
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
  for (;;) {
    const ilda_header_t *header = ilda_read_next_header(&ilda);
    if (header == NULL) {
      fprintf(stderr, "error when reading ILDA file: %s\n", ilda.error);
      exit(1);
    }
    printf("Header found of type %d (%s):\n", header->format_code, 
        ilda_is_palette(header) ? "palette" : "frame");
    if (ilda_is_end_of_file(header)) {
      printf("  [END OF FILE]\n");
      break;
    }
    printf("  - frame or color palette name: %s\n", header->frame_or_color_palette_name);
    printf("  - company name: %s\n", header->company_name);
    printf("  - number of records: %d\n", header->number_of_records);
    printf("  - frame or color palette number: %d\n", header->frame_or_color_palette_number);
    printf("  - total frames: %d\n", header->total_frames);
    printf("  - projector number: %d\n", header->projector_number);
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
      for (size_t i = 0; i < header->number_of_records; i++) {
        const ilda_point_t *point = &buffer[i];
        printf("Record %ld\n", i+1);
        printf("  - pos: (%d, %d, %d)\n", point->pos.x, point->pos.y, point->pos.z);
        printf("  - color: (%d, %d, %d)\n", point->color.r, point->color.g, point->color.b);
        printf("  - status: laser %s%s\n", ilda_is_blanking(point->status_code) ? "off" : "on",
            ilda_is_last_point(point->status_code) ? " / last point" : "");
      }
    }
    printf("\n");
  }
}
