#ifndef ILDA_DECODER_H
#define ILDA_DECODER_H

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

// ILDA_OK is 0, ILDA_ERR is 1
typedef enum {
  ILDA_OK,
  ILDA_ERR,
} ilda_status;

typedef struct {
  int16_t x, y, z;
} ilda_pos_t;

typedef struct {
  uint8_t r, g, b;
} ilda_color_t;

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
  uint8_t strict_mode;
} ilda_state_t;

// Initialize a ILDA state from a callback read function and an opaque value.
// The opaque value will be given as the callback first parameter.
// The strict mode enforces the ILDA standard when reading a file, while
// the non-strict mode is more lax with the input.
void ilda_init(ilda_state_t *ilda,
               ssize_t (*read)(void *opaque, void *buffer, size_t len),
               void *opaque, bool strict_mode);

// Read the next header. Return a pointer to the read-only
// header, or NULL if there was an error. In case of error,
// an error message can be found in ilda->error.
const ilda_header_t *ilda_read_next_header(ilda_state_t *ilda);

// Return true if the given header denotes a palette, 0 otherwise.
bool ilda_is_palette(const ilda_header_t *header);

// Return true if we have reached the end of the file.
bool ilda_is_end_of_file(const ilda_header_t *header);

// Read the palette, return ILDA_ERROR if there was an error,
// with an error message in ilda->error.
ilda_status ilda_read_palette(ilda_state_t *ilda);

// Read the records. The buffer shall be large enough to contain the
// right number of records. Return ILDA_ERROR if there was an error,
// with an error message in ilda->error. len is in bytes.
ilda_status ilda_read_records(ilda_state_t *ilda, ilda_point_t *points,
                              size_t len);

// Return true if the status represents blanking (laser off), 0 otherwise.
bool ilda_is_blanking(uint8_t status);

// Return true if the status represents the last point of the image, 0
// otherwise.
bool ilda_is_last_point(uint8_t status);

#endif // ILDA_DECODER_H
