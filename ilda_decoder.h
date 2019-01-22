#ifndef ILDA_DECODER_H
#define ILDA_DECODER_H

#include <stdint.h>
#include <unistd.h>

typedef struct { int16_t x, y, z; } ilda_pos_t;

typedef struct { uint8_t r, g, b; } ilda_color_t;

typedef struct {
  // read(opaque, buffer, length) returns the number of bytes
  // read (which should not exceed length). 0 means end-of-file,
  // < 0 means error.
  ssize_t (*read)(void *, void *, size_t);
  void *opaque;
} ilda_provider_t;

typedef struct {
  ilda_pos_t pos;
  ilda_color_t color;
  uint8_t status_code;
} ilda_point_t;

typedef struct {
  uint8_t format_code;
  char frame_or_color_palette_name[9];
  char company_name[9];
  uint16_t number_of_records;
  uint16_t frame_or_color_palette_number;
  uint16_t total_frames;
  uint8_t projector_number;
} ilda_header_t;

typedef struct {
  ilda_provider_t provider;
  ilda_color_t palette[256];
  ilda_header_t current_header;
  const char *error;
} ilda_state_t;

// Initialize a ILDA state from a read function and an opaque value.
void ilda_init(ilda_state_t *ilda,
               ssize_t (*read)(void *opaque, void *buffer, size_t len),
               void *opaque);

// Read the next header. Return a pointer to the read-only
// header, or NULL if there was an error. In case of error,
// an error message can be found in ilda->error.
const ilda_header_t *ilda_read_next_header(ilda_state_t *ilda);

// Return 1 if the given header denotes a palette, 0 otherwise.
int ilda_is_palette(const ilda_header_t *header);

// Return 1 if we have reached the end of the file.
int ilda_is_end_of_file(const ilda_header_t *header);

// Read the palette, return non-zero if there was an error,
// with an error message in ilda->error.
int ilda_read_palette(ilda_state_t *ilda);

// Read the records. The buffer shall be large enough to contain the
// right number of records. Return non-zero if there was an error,
// with an error message in ilda->error. len is in bytes.
int ilda_read_records(ilda_state_t *ilda, ilda_point_t *points, size_t len);

// Return 1 if the status represents blanking (laser off), 0 otherwise.
static inline int ilda_is_blanking(uint8_t status) {
  return (status & 0x40) != 0;
}

// Return 1 if the status represents the last point of the image, 0 otherwise.
static inline int ilda_is_last_point(uint8_t status) {
  return (status & 0x80) != 0;
}

#endif // ILDA_DECODER_H
